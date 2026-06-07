/**
 * Generic LSP client plugin for qi.
 *
 * Supports any LSP-compatible language server. Built-in configs cover the
 * most common ones; adding a new language is a one-line entry in
 * lsp_lang_configs[].
 *
 * Key bindings (global):
 *   M-?   lsp-hover     — show hover info for symbol at cursor
 *   M-/   lsp-complete  — show completion list at cursor
 *
 * Debug log: set LSP_DEBUG to 1, then tail /tmp/qi_lsp.log
 */
#include "qe.h"
#include "lsp.h"
#include "cJSON.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LSP_DEBUG 0
#if LSP_DEBUG
#define LSP_LOG(fmt, ...) do { \
    FILE *_lf = fopen("/tmp/qi_lsp.log", "a"); \
    if (_lf) { fprintf(_lf, fmt "\n", ##__VA_ARGS__); fclose(_lf); } \
} while(0)
#else
#define LSP_LOG(fmt, ...)
#endif

/* -------------------------------------------------------------------------
 * Built-in language server configurations.
 * Extend this table to add new languages — no other code changes needed.
 * ------------------------------------------------------------------------- */
static const LSPLangConfig lsp_lang_configs[] = {
    { "go",   "go",         "gopls",                      {"serve", NULL} },
    { "rs",   "rust",       "rust-analyzer",              {NULL} },
    { "py",   "python",     "pylsp",                      {NULL} },
    { "c",    "c",          "clangd",                     {NULL} },
    { "cpp",  "cpp",        "clangd",                     {NULL} },
    { "h",    "c",          "clangd",                     {NULL} },
    { "js",   "javascript", "typescript-language-server", {"--stdio", NULL} },
    { "ts",   "typescript", "typescript-language-server", {"--stdio", NULL} },
    { "lua",  "lua",        "lua-language-server",        {NULL} },
    { NULL }
};

LSPServer lsp_servers[LSP_MAX_SERVERS];

static ModeDef lsp_completion_mode; /* initialised in lsp_init */

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */

static void lsp_file_uri(const char *path, char *buf, int buf_size)
{
    snprintf(buf, buf_size, "file://%s", path);
}

static void lsp_dir_from_path(const char *path, char *buf, int buf_size)
{
    const char *p = strrchr(path, '/');
    if (p && p != path) {
        snprintf(buf, buf_size, "%.*s", (int)(p - path), path);
    } else {
        getcwd(buf, buf_size);
    }
}

static void lsp_send_raw(LSPServer *srv, const char *json)
{
    int len = strlen(json);
    char header[64];
    snprintf(header, sizeof(header), "Content-Length: %d\r\n\r\n", len);
    write(srv->stdin_fd, header, strlen(header));
    write(srv->stdin_fd, json, len);
}

static void lsp_send_obj(LSPServer *srv, cJSON *root)
{
    char *json = cJSON_PrintUnformatted(root);
    if (json) {
        lsp_send_raw(srv, json);
        free(json);
    }
    cJSON_Delete(root);
}

/* -------------------------------------------------------------------------
 * LSP protocol messages
 * ------------------------------------------------------------------------- */

static void lsp_send_initialize(LSPServer *srv, const char *file_path)
{
    char dir[MAX_FILENAME_SIZE];
    char root_uri[MAX_FILENAME_SIZE + 10];

    lsp_dir_from_path(file_path, dir, sizeof(dir));
    lsp_file_uri(dir, root_uri, sizeof(root_uri));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    srv->init_id = srv->next_id++;
    cJSON_AddNumberToObject(root, "id", srv->init_id);
    cJSON_AddStringToObject(root, "method", "initialize");

    cJSON *params = cJSON_CreateObject();
    cJSON_AddNumberToObject(params, "processId", getpid());
    cJSON_AddStringToObject(params, "rootUri", root_uri);
    cJSON_AddItemToObject(params, "capabilities", cJSON_CreateObject());
    cJSON_AddItemToObject(root, "params", params);

    lsp_send_obj(srv, root);
}

static void lsp_send_initialized(LSPServer *srv)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddStringToObject(root, "method", "initialized");
    cJSON_AddItemToObject(root, "params", cJSON_CreateObject());
    lsp_send_obj(srv, root);
}

static void lsp_send_did_open(LSPServer *srv, const char *path,
                              const char *language_id)
{
    char uri[MAX_FILENAME_SIZE + 10];
    lsp_file_uri(path, uri, sizeof(uri));

    /* read file content from disk */
    FILE *f = fopen(path, "r");
    if (!f)
        return;
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize > 512 * 1024)
        fsize = 512 * 1024;
    char *content = malloc(fsize + 1);
    if (!content) {
        fclose(f);
        return;
    }
    if (fread(content, 1, fsize, f) != (size_t)fsize) {
        free(content);
        fclose(f);
        return;
    }
    content[fsize] = '\0';
    fclose(f);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddStringToObject(root, "method", "textDocument/didOpen");

    cJSON *params  = cJSON_CreateObject();
    cJSON *textDoc = cJSON_CreateObject();
    cJSON_AddStringToObject(textDoc, "uri",        uri);
    cJSON_AddStringToObject(textDoc, "languageId", language_id);
    cJSON_AddNumberToObject (textDoc, "version",    1);
    cJSON_AddStringToObject(textDoc, "text",       content);
    free(content);

    cJSON_AddItemToObject(params, "textDocument", textDoc);
    cJSON_AddItemToObject(root,   "params",       params);
    lsp_send_obj(srv, root);

    /* record this file as open */
    if (srv->num_open_files < LSP_MAX_OPEN_FILES) {
        pstrcpy(srv->open_files[srv->num_open_files].uri,
                sizeof(srv->open_files[0].uri), uri);
        srv->open_files[srv->num_open_files].version = 1;
        srv->num_open_files++;
    }
}

/* Check whether this file has already been opened with the server. */
static int lsp_file_is_open(LSPServer *srv, const char *uri)
{
    for (int i = 0; i < srv->num_open_files; i++) {
        if (strcmp(srv->open_files[i].uri, uri) == 0)
            return 1;
    }
    return 0;
}

/* Public: ensure the server has received didOpen for this file. */
void lsp_ensure_did_open(LSPServer *srv, EditState *s)
{
    if (!srv->initialized)
        return;

    char uri[MAX_FILENAME_SIZE + 10];
    lsp_file_uri(s->b->filename, uri, sizeof(uri));

    if (!lsp_file_is_open(srv, uri))
        lsp_send_did_open(srv, s->b->filename, srv->config->language_id);
}

/* -------------------------------------------------------------------------
 * Response handlers
 * ------------------------------------------------------------------------- */

static void lsp_handle_hover(cJSON *result, LSPServer *srv)
{
    QEmacsState *qs = &qe_state;
    EditState   *es = srv->hover_es ? srv->hover_es : qs->active_window;
    if (!es)
        return;

    if (!result || cJSON_IsNull(result)) {
        put_status(es, "LSP: no hover info at cursor");
        return;
    }

    cJSON      *contents = cJSON_GetObjectItem(result, "contents");
    const char *text     = NULL;

    if (cJSON_IsObject(contents)) {
        cJSON *value = cJSON_GetObjectItem(contents, "value");
        if (value && cJSON_IsString(value))
            text = value->valuestring;
    } else if (cJSON_IsString(contents)) {
        text = contents->valuestring;
    } else if (cJSON_IsArray(contents)) {
        /* take the first item */
        cJSON *first = contents->child;
        if (first && cJSON_IsString(first))
            text = first->valuestring;
        else if (first && cJSON_IsObject(first)) {
            cJSON *v = cJSON_GetObjectItem(first, "value");
            if (v && cJSON_IsString(v))
                text = v->valuestring;
        }
    }

    if (!text || !*text) {
        put_status(es, "LSP: hover returned no text");
        return;
    }

    /* LSP hover content is markdown. Strip code fences (```lang ... ```)
     * and collect the first non-empty, non-fence line for the status bar,
     * then show the full cleaned text in a popup. */
    static char status_line[256];
    EditBuffer *b = eb_find("*lsp-hover*");
    if (!b)
        b = eb_new("*lsp-hover*", BF_SYSTEM);
    else {
        b->flags &= ~BF_READONLY;
        eb_delete(b, 0, b->total_size);
    }

    status_line[0] = '\0';
    const char *p = text;
    while (*p) {
        /* find end of current line */
        const char *eol = strchr(p, '\n');
        int line_len = eol ? (int)(eol - p) : (int)strlen(p);

        /* skip code fence markers (``` lines) */
        if (line_len >= 3 && p[0] == '`' && p[1] == '`' && p[2] == '`') {
            p = eol ? eol + 1 : p + line_len;
            continue;
        }
        /* skip horizontal rule (--- or ===) */
        if (line_len >= 3 && (p[0] == '-' || p[0] == '=') &&
            p[0] == p[1] && p[1] == p[2]) {
            p = eol ? eol + 1 : p + line_len;
            continue;
        }

        /* write this line to the popup buffer */
        eb_insert(b, b->total_size, p, line_len);
        eb_insert(b, b->total_size, "\n", 1);

        /* capture first non-empty line for the status bar */
        if (!status_line[0] && line_len > 0) {
            int n = line_len < (int)sizeof(status_line) - 1
                    ? line_len : (int)sizeof(status_line) - 1;
            memcpy(status_line, p, n);
            status_line[n] = '\0';
        }

        p = eol ? eol + 1 : p + line_len;
    }

    b->flags |= BF_READONLY;

    if (status_line[0])
        put_status(es, "%s", status_line);
    else
        put_status(es, "LSP: hover (see *lsp-hover* buffer)");

    show_popup(b);
}

/* -------------------------------------------------------------------------
 * Completion dropdown helpers
 * ------------------------------------------------------------------------- */

static int is_window_alive(EditState *target)
{
    QEmacsState *qs = &qe_state;
    EditState *w;
    for (w = qs->first_window; w; w = w->next_window)
        if (w == target) return 1;
    return 0;
}

static LSPServer *lsp_completion_find_server(EditState *popup)
{
    for (int i = 0; i < LSP_MAX_SERVERS; i++)
        if (lsp_servers[i].completion_popup == popup)
            return &lsp_servers[i];
    return NULL;
}

static void lsp_completion_close(LSPServer *srv, EditState *restore_to)
{
    QEmacsState *qs = &qe_state;
    if (!srv->completion_popup)
        return;
    EditBuffer *pb = srv->completion_popup->b;
    edit_close(srv->completion_popup);
    eb_free(pb);
    srv->completion_popup = NULL;
    if (restore_to && is_window_alive(restore_to))
        qs->active_window = restore_to;
    do_refresh(qs->active_window);
}

static void lsp_completion_abort(EditState *s)
{
    LSPServer *srv = lsp_completion_find_server(s);
    if (srv)
        lsp_completion_close(srv, srv->completion_target_es);
}

static void lsp_completion_next(EditState *s) { do_up_down(s,  1); }
static void lsp_completion_prev(EditState *s) { do_up_down(s, -1); }

static void lsp_completion_confirm(EditState *s)
{
    LSPServer *srv = lsp_completion_find_server(s);
    if (!srv)
        return;

    EditState *tes = srv->completion_target_es;
    if (!tes || !is_window_alive(tes)) {
        lsp_completion_close(srv, NULL);
        return;
    }

    /* extract insertion text from selected line: " insertText\tdetail"
     * Skip first char — it's the list_mode selection marker (' ' or '*') */
    int off = list_get_offset(s);
    char linebuf[512];
    eb_get_strline(s->b, linebuf, sizeof(linebuf), &off);
    char *label = linebuf + 1;
    char *tab = strchr(label, '\t');
    if (tab) *tab = '\0';
    int label_len = strlen(label);

    if (label_len == 0) {
        lsp_completion_close(srv, tes);
        return;
    }

    /* replace the partial word with the selected insertion text */
    int word_start = srv->completion_word_start;
    int word_end   = tes->offset;
    if (word_end > word_start)
        eb_delete(tes->b, word_start, word_end - word_start);
    eb_insert(tes->b, word_start, label, label_len);
    tes->offset = word_start + label_len;

    lsp_completion_close(srv, tes);
}

/* Rebuild the visible popup by prefix-filtering the master buffer against
 * the current partial word in the editor (word_start..tes->offset). */
static void lsp_completion_refilter(LSPServer *srv)
{
    QEmacsState *qs = &qe_state;
    if (!srv->completion_popup) return;

    EditState *tes = srv->completion_target_es;
    if (!tes || !is_window_alive(tes)) return;

    /* current partial word */
    char filter[256] = {0};
    int flen = tes->offset - srv->completion_word_start;
    if (flen < 0) flen = 0;
    if (flen >= (int)sizeof(filter)) flen = (int)sizeof(filter) - 1;
    if (flen > 0)
        eb_read(tes->b, srv->completion_word_start, (unsigned char *)filter, flen);

    EditBuffer *all_b = eb_find("*lsp-completions-all*");
    if (!all_b) return;

    EditBuffer *pop_b = srv->completion_popup->b;
    pop_b->flags &= ~BF_READONLY;
    eb_delete(pop_b, 0, pop_b->total_size);

    int off = 0;
    char linebuf[512];
    int count = 0;
    while (off < all_b->total_size) {
        int old_off = off;
        eb_get_strline(all_b, linebuf, sizeof(linebuf), &off);
        if (off <= old_off) break;

        char *sym = linebuf + 1;  /* skip list_mode marker */
        char *tab = strchr(sym, '\t');
        char saved = 0;
        if (tab) { saved = *tab; *tab = '\0'; }

        /* case-insensitive prefix match */
        int match = (flen == 0);
        if (!match) {
            int i;
            for (i = 0; i < flen && sym[i] && filter[i]; i++) {
                if (tolower((unsigned char)sym[i]) != tolower((unsigned char)filter[i]))
                    break;
            }
            match = (i == flen);
        }

        if (tab) *tab = saved;

        if (match) {
            eb_printf(pop_b, "%s\n", linebuf);
            count++;
        }
    }

    if (count == 0)
        eb_printf(pop_b, " (no matches)\n");

    pop_b->flags |= BF_READONLY;
    srv->completion_popup->offset = 0;
    srv->completion_popup->force_highlight = 1;

    edit_display(qs);
    dpy_flush(qs->screen);
}

/* Typed char while popup is focused: insert into editor buffer and refilter. */
static void lsp_completion_char(EditState *s, int key)
{
    LSPServer *srv = lsp_completion_find_server(s);
    if (!srv) return;

    EditState *tes = srv->completion_target_es;
    if (!tes || !is_window_alive(tes)) return;

    char buf[MAX_CHAR_BYTES] = {0};
    to_utf8(buf, key);
    int len = utf8_len(buf[0]);
    eb_insert(tes->b, tes->offset, buf, len);
    tes->offset += len;

    lsp_completion_refilter(srv);
}

/* Backspace while popup is focused: delete last typed char and refilter. */
static void lsp_completion_backspace(EditState *s)
{
    LSPServer *srv = lsp_completion_find_server(s);
    if (!srv) return;

    EditState *tes = srv->completion_target_es;
    if (!tes || !is_window_alive(tes)) return;

    if (tes->offset <= srv->completion_word_start) return;

    int prev;
    eb_prevc(tes->b, tes->offset, &prev);
    eb_delete(tes->b, prev, tes->offset - prev);
    tes->offset = prev;

    lsp_completion_refilter(srv);
}

static CmdDef lsp_completion_commands[] = {
    CMD0(KEY_CTRL('n'), KEY_DOWN,      "lsp-completion-next",      lsp_completion_next)
    CMD0(KEY_CTRL('p'), KEY_UP,        "lsp-completion-prev",      lsp_completion_prev)
    CMD0(KEY_RET,       KEY_NONE,      "lsp-completion-confirm",   lsp_completion_confirm)
    CMD0(KEY_ESC,       KEY_CTRL('g'), "lsp-completion-abort",     lsp_completion_abort)
    CMD0(KEY_DEL,       KEY_CTRL('h'), "lsp-completion-backspace", lsp_completion_backspace)
    CMDV(KEY_DEFAULT,   KEY_NONE,      "lsp-completion-char",      lsp_completion_char, 0, "v")
    CMD_DEF_END,
};

static void lsp_handle_completion(cJSON *result, LSPServer *srv)
{
    QEmacsState *qs  = &qe_state;
    EditState   *tes = srv->completion_target_es;

    /* fall back to static popup if there is no valid target window */
    if (!tes || !is_window_alive(tes))
        tes = qs->active_window;
    if (!tes)
        return;

    cJSON *items = NULL;
    if (cJSON_IsObject(result)) {
        items = cJSON_GetObjectItem(result, "items");
    } else if (cJSON_IsArray(result)) {
        items = result;
    }

    if (!items || cJSON_GetArraySize(items) == 0) {
        put_status(tes, "LSP: no completions");
        return;
    }

    /* get cursor pixel position for popup placement */
    CursorContext cm;
    cm.xc = NO_CURSOR;
    cm.cursor_height = 0;
    get_cursor_pos(tes, &cm);

    /* reuse or create the completions buffer */
    EditBuffer *b = eb_find("*lsp-completions*");
    if (!b) {
        b = eb_new("*lsp-completions*", BF_SYSTEM);
    } else {
        b->flags &= ~BF_READONLY;
        eb_delete(b, 0, b->total_size);
    }

    /* master copy — kept unfiltered so typing can re-filter without re-querying LSP */
    EditBuffer *all_b = eb_find("*lsp-completions-all*");
    if (!all_b) {
        all_b = eb_new("*lsp-completions-all*", BF_SYSTEM);
    } else {
        all_b->flags &= ~BF_READONLY;
        eb_delete(all_b, 0, all_b->total_size);
    }

    /* one item per line: " insertText\tdetail" (leading space = list_mode marker)
     * Use insertText when clangd provides it — it's always the clean symbol name.
     * Fall back to label, trimming any leading whitespace clangd sometimes adds. */
    int n_items = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, items) {
        cJSON *lbl = cJSON_GetObjectItem(item, "label");
        cJSON *ins = cJSON_GetObjectItem(item, "insertText");
        cJSON *det = cJSON_GetObjectItem(item, "detail");
        if (!lbl || !cJSON_IsString(lbl))
            continue;
        const char *text = lbl->valuestring;
        if (ins && cJSON_IsString(ins) && ins->valuestring[0])
            text = ins->valuestring;
        while (*text == ' ' || *text == '\t') text++;
        if (*text == '\0')
            continue;
        if (det && cJSON_IsString(det)) {
            eb_printf(b,     " %s\t%s\n", text, det->valuestring);
            eb_printf(all_b, " %s\t%s\n", text, det->valuestring);
        } else {
            eb_printf(b,     " %s\n", text);
            eb_printf(all_b, " %s\n", text);
        }
        n_items++;
    }
    b->flags     |= BF_READONLY;
    all_b->flags |= BF_READONLY;

    if (n_items == 0) {
        put_status(tes, "LSP: no completions");
        return;
    }

    /* if cursor position is unknown, fall back to centred show_popup */
    if (cm.xc == NO_CURSOR || cm.cursor_height <= 0) {
        show_popup(b);
        return;
    }

    /* compute popup geometry */
    int line_h  = cm.cursor_height;
    int char_w  = line_h / 2 > 0 ? line_h / 2 : 1;
    int popup_h = (n_items < 8 ? n_items : 8) * line_h;
    int popup_w = 50 * char_w;
    int scr_w   = qs->screen->width;
    int scr_h   = qs->screen->height - qs->status_height;

    int x = cm.xc;
    if (x + popup_w > scr_w) x = scr_w - popup_w;
    if (x < 0) x = 0;

    int y = cm.yc + cm.cursor_height;
    if (y + popup_h > scr_h)
        y = cm.yc - popup_h;   /* show above cursor instead */
    if (y < 0) y = 0;

    /* close any stale popup before creating a new one */
    if (srv->completion_popup)
        lsp_completion_close(srv, tes);

    EditState *e = edit_new(b, x, y, popup_w, popup_h, WF_POPUP);
    do_set_mode(e, &lsp_completion_mode, NULL);
    e->force_highlight = 1;
    e->offset = 0;
    srv->completion_popup = e;
    qs->active_window = e;
    do_refresh(e);
    /* force immediate render — edit_display+dpy_flush only fire in the key
     * handler normally, so the popup would be invisible until the next
     * keypress without this explicit flush. */
    edit_display(qs);
    dpy_flush(qs->screen);
}

/* -------------------------------------------------------------------------
 * Message dispatch (called with a complete JSON-RPC body)
 * ------------------------------------------------------------------------- */

static void lsp_dispatch(LSPServer *srv, const char *json_str)
{
    LSP_LOG("lsp_dispatch: %.120s", json_str);

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        LSP_LOG("lsp_dispatch: cJSON_Parse failed");
        return;
    }

    cJSON *id_item = cJSON_GetObjectItem(root, "id");
    cJSON *result  = cJSON_GetObjectItem(root, "result");
    cJSON *method  = cJSON_GetObjectItem(root, "method");

    LSP_LOG("lsp_dispatch: id=%s result=%s method=%s",
            id_item ? "yes" : "no",
            result  ? "yes" : "no",
            (method && cJSON_IsString(method)) ? method->valuestring : "none");

    if (id_item && cJSON_IsNumber(id_item) && result) {
        int id = (int)id_item->valuedouble;
        LSP_LOG("lsp_dispatch: response id=%d init_id=%d hover_id=%d",
                id, srv->init_id, srv->hover_id);

        if (id == srv->init_id) {
            LSP_LOG("lsp_dispatch: initialize complete");
            lsp_send_initialized(srv);
            srv->initialized = 1;
            /* send any file that was queued before init completed */
            if (srv->open_on_init[0]) {
                LSP_LOG("lsp_dispatch: sending didOpen for %s", srv->open_on_init);
                lsp_send_did_open(srv, srv->open_on_init,
                                  srv->config->language_id);
            }
        } else if (id == srv->hover_id) {
            LSP_LOG("lsp_dispatch: hover response received");
            lsp_handle_hover(result, srv);
            srv->hover_id = -1;
            srv->hover_es = NULL;
        } else if (id == srv->completion_id) {
            LSP_LOG("lsp_dispatch: completion response received");
            lsp_handle_completion(result, srv);
            srv->completion_id = -1;
        }
    }
    /* server-to-client notifications (no id) are silently ignored */

    cJSON_Delete(root);
}

/* -------------------------------------------------------------------------
 * Async I/O
 * ------------------------------------------------------------------------- */

static void lsp_read_handler(void *opaque)
{
    LSPServer *srv   = opaque;
    int        space = sizeof(srv->read_buf) - srv->read_len - 1;
    if (space <= 0) {
        srv->read_len = 0; /* discard on overflow */
        return;
    }

    int n = read(srv->stdout_fd, srv->read_buf + srv->read_len, space);
    LSP_LOG("lsp_read_handler: read %d bytes (fd=%d)", n, srv->stdout_fd);
    if (n <= 0)
        return;
    srv->read_len += n;
    srv->read_buf[srv->read_len] = '\0';

    char *buf       = srv->read_buf;
    int   remaining = srv->read_len;

    while (remaining > 0) {
        char *header_end = strstr(buf, "\r\n\r\n");
        if (!header_end)
            break;

        int content_length = 0;
        if (sscanf(buf, "Content-Length: %d", &content_length) != 1)
            break;

        int header_len = (header_end + 4) - buf;
        if (remaining < header_len + content_length)
            break;

        /* temporarily null-terminate the body */
        char saved = buf[header_len + content_length];
        buf[header_len + content_length] = '\0';
        lsp_dispatch(srv, buf + header_len);
        buf[header_len + content_length] = saved;

        int consumed = header_len + content_length;
        buf       += consumed;
        remaining -= consumed;
    }

    if (remaining < srv->read_len) {
        memmove(srv->read_buf, buf, remaining);
        srv->read_len = remaining;
    }
}

static void lsp_server_exit(void *opaque, int status)
{
    LSPServer   *srv = opaque;
    QEmacsState *qs  = &qe_state;

    srv->active      = 0;
    srv->initialized = 0;

    if (srv->completion_popup)
        lsp_completion_close(srv, srv->completion_target_es);

    if (qs->active_window)
        put_status(qs->active_window, "LSP (%s): server exited",
                   srv->config ? srv->config->cmd : "?");
}

/* -------------------------------------------------------------------------
 * Server lifecycle
 * ------------------------------------------------------------------------- */

static LSPServer *lsp_find_free_slot(void)
{
    for (int i = 0; i < LSP_MAX_SERVERS; i++) {
        if (!lsp_servers[i].active)
            return &lsp_servers[i];
    }
    return NULL;
}

static LSPServer *lsp_find_running(const LSPLangConfig *cfg)
{
    for (int i = 0; i < LSP_MAX_SERVERS; i++) {
        if (lsp_servers[i].active && lsp_servers[i].config == cfg)
            return &lsp_servers[i];
    }
    return NULL;
}

static LSPServer *lsp_start_server(const LSPLangConfig *cfg, EditState *s)
{
    LSPServer *srv = lsp_find_free_slot();
    if (!srv) {
        put_status(s, "LSP: too many servers running");
        return NULL;
    }

    int in_pipe[2], out_pipe[2];
    if (pipe(in_pipe) < 0) {
        put_status(s, "LSP: pipe() failed");
        return NULL;
    }
    if (pipe(out_pipe) < 0) {
        close(in_pipe[0]); close(in_pipe[1]);
        put_status(s, "LSP: pipe() failed");
        return NULL;
    }

    int pid = fork();
    if (pid < 0) {
        put_status(s, "LSP: fork() failed");
        close(in_pipe[0]);  close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);
        return NULL;
    }

    if (pid == 0) {
        /* child: exec the language server */
        dup2(in_pipe[0],  STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        close(in_pipe[0]);  close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);
        /* silence stderr so it doesn't pollute the tty */
        int null_fd = open("/dev/null", O_WRONLY);
        if (null_fd >= 0) {
            dup2(null_fd, STDERR_FILENO);
            close(null_fd);
        }

        /* build argv: cmd + args + NULL */
        const char *argv[LSP_MAX_CMD_ARGS + 2];
        argv[0] = cfg->cmd;
        int ai = 1;
        for (int i = 0; i < LSP_MAX_CMD_ARGS && cfg->args[i]; i++)
            argv[ai++] = cfg->args[i];
        argv[ai] = NULL;

        execvp(cfg->cmd, (char *const *)argv);
        exit(1);
    }

    /* parent */
    close(in_pipe[0]);
    close(out_pipe[1]);

    memset(srv, 0, sizeof(*srv));
    srv->active        = 1;
    srv->initialized   = 0;
    srv->stdin_fd      = in_pipe[1];
    srv->stdout_fd     = out_pipe[0];
    srv->pid           = pid;
    srv->next_id       = 1;
    srv->hover_id      = -1;
    srv->completion_id = -1;
    srv->config        = cfg;
    pstrcpy(srv->open_on_init, sizeof(srv->open_on_init), s->b->filename);

    set_read_handler(srv->stdout_fd, lsp_read_handler, srv);
    set_pid_handler(pid, lsp_server_exit, srv);

    LSP_LOG("lsp_start_server: pid=%d stdin_fd=%d stdout_fd=%d file=%s",
            pid, srv->stdin_fd, srv->stdout_fd, s->b->filename);

    lsp_send_initialize(srv, s->b->filename);

    put_status(s, "LSP: starting %s...", cfg->cmd);
    return srv;
}

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */

LSPServer *lsp_get_server_for_file(EditState *s)
{
    if (!s || !s->b || !s->b->filename[0])
        return NULL;

    const char *ext = extension(s->b->filename);
    if (!ext || !*ext)
        return NULL;
    /* extension() includes the leading dot */
    const char *bare_ext = (*ext == '.') ? ext + 1 : ext;

    const LSPLangConfig *cfg = NULL;
    for (int i = 0; lsp_lang_configs[i].ext; i++) {
        if (strcmp(lsp_lang_configs[i].ext, bare_ext) == 0) {
            cfg = &lsp_lang_configs[i];
            break;
        }
    }
    if (!cfg)
        return NULL;

    LSPServer *srv = lsp_find_running(cfg);
    if (!srv)
        srv = lsp_start_server(cfg, s);
    return srv;
}

void lsp_hover(EditState *s)
{
    LSPServer *srv = lsp_get_server_for_file(s);
    if (!srv) {
        put_status(s, "LSP: no server for this file type");
        return;
    }
    if (!srv->initialized) {
        put_status(s, "LSP: server not ready yet, try again shortly");
        return;
    }

    lsp_ensure_did_open(srv, s);

    char uri[MAX_FILENAME_SIZE + 10];
    lsp_file_uri(s->b->filename, uri, sizeof(uri));

    int line, col;
    eb_get_pos(s->b, &line, &col, s->offset);

    LSP_LOG("lsp_hover: sending hover uri=%s line=%d col=%d", uri, line, col);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    int id = srv->next_id++;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "method", "textDocument/hover");

    cJSON *params  = cJSON_CreateObject();
    cJSON *textDoc = cJSON_CreateObject();
    cJSON_AddStringToObject(textDoc, "uri", uri);
    cJSON_AddItemToObject(params, "textDocument", textDoc);

    cJSON *pos = cJSON_CreateObject();
    cJSON_AddNumberToObject(pos, "line",      line);
    cJSON_AddNumberToObject(pos, "character", col);
    cJSON_AddItemToObject(params, "position", pos);
    cJSON_AddItemToObject(root,   "params",   params);

    srv->hover_id = id;
    srv->hover_es = s;
    lsp_send_obj(srv, root);
}

void lsp_complete(EditState *s)
{
    LSPServer *srv = lsp_get_server_for_file(s);
    if (!srv) {
        put_status(s, "LSP: no server for this file type");
        return;
    }
    if (!srv->initialized) {
        put_status(s, "LSP: server not ready yet, try again shortly");
        return;
    }

    /* close any stale dropdown before sending a fresh request */
    if (srv->completion_popup)
        lsp_completion_close(srv, srv->completion_target_es);

    /* capture the target window and the start of the partial word */
    srv->completion_target_es = s;
    int word_start = s->offset;
    int prev;
    while (word_start > 0) {
        int c = eb_prevc(s->b, word_start, &prev);
        if (!isalnum(c) && c != '_')
            break;
        word_start = prev;
    }
    /* if no partial identifier was found, also eat any adjacent whitespace
     * so that completing after "ptr-> " inserts right after "->" */
    if (word_start == s->offset) {
        int ws = s->offset;
        while (ws > 0) {
            int c = eb_prevc(s->b, ws, &prev);
            if (c != ' ' && c != '\t')
                break;
            ws = prev;
        }
        word_start = ws;
    }
    srv->completion_word_start = word_start;

    lsp_ensure_did_open(srv, s);

    char uri[MAX_FILENAME_SIZE + 10];
    lsp_file_uri(s->b->filename, uri, sizeof(uri));

    int line, col;
    eb_get_pos(s->b, &line, &col, s->offset);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    int id = srv->next_id++;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddStringToObject(root, "method", "textDocument/completion");

    cJSON *params  = cJSON_CreateObject();
    cJSON *textDoc = cJSON_CreateObject();
    cJSON_AddStringToObject(textDoc, "uri", uri);
    cJSON_AddItemToObject(params, "textDocument", textDoc);

    cJSON *pos = cJSON_CreateObject();
    cJSON_AddNumberToObject(pos, "line",      line);
    cJSON_AddNumberToObject(pos, "character", col);
    cJSON_AddItemToObject(params, "position", pos);
    cJSON_AddItemToObject(root,   "params",   params);

    srv->completion_id = id;
    lsp_send_obj(srv, root);
}

/* -------------------------------------------------------------------------
 * Command wrappers and plugin init
 * ------------------------------------------------------------------------- */

static void do_lsp_hover(EditState *s)    { lsp_hover(s); }
static void do_lsp_complete(EditState *s) { lsp_complete(s); }

static CmdDef lsp_commands[] = {
    CMD0(KEY_META('?'), KEY_NONE, "lsp-hover",    do_lsp_hover)
    CMD0(KEY_META('/'), KEY_NONE, "lsp-complete",  do_lsp_complete)
    CMD_DEF_END,
};

int lsp_init(void)
{
    memset(lsp_servers, 0, sizeof(lsp_servers));

    /* build lsp_completion_mode from list_mode, add its key bindings */
    memcpy(&lsp_completion_mode, &list_mode, sizeof(ModeDef));
    lsp_completion_mode.name       = "lsp-completion";
    lsp_completion_mode.mode_flags = MODEF_NOCMD;
    qe_register_mode(&lsp_completion_mode);
    qe_register_cmd_table(lsp_completion_commands, "lsp-completion");

    /* register global LSP commands (NULL mode = available everywhere) */
    qe_register_cmd_table(lsp_commands, NULL);
    return 0;
}

