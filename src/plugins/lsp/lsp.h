#ifndef LSP_H
#define LSP_H

/* qe.h is included by qe.c before plugincore.h (which aggregates all plugin
 * headers including this one). Do not #include "qe.h" here directly. */
#ifndef QE_H
/* Forward declaration — resolved when qe.h is included first. */
typedef struct EditState EditState;
#endif

#ifndef MAX_FILENAME_SIZE
#define MAX_FILENAME_SIZE 1024
#endif

#define LSP_MAX_SERVERS    8
#define LSP_MAX_OPEN_FILES 32
#define LSP_READ_BUF_SIZE  (128 * 1024)
#define LSP_MAX_CMD_ARGS   8

typedef struct {
    const char *ext;         /* file extension without dot, e.g. "go" */
    const char *language_id; /* LSP languageId, e.g. "go" */
    const char *cmd;         /* server executable */
    const char *args[LSP_MAX_CMD_ARGS]; /* extra args, NULL-terminated */
} LSPLangConfig;

typedef struct {
    char uri[MAX_FILENAME_SIZE + 10];
    int  version;
} LSPOpenFile;

typedef struct {
    int active;
    int initialized;
    int stdin_fd;
    int stdout_fd;
    int pid;

    char read_buf[LSP_READ_BUF_SIZE];
    int  read_len;

    int next_id;
    int init_id;
    int hover_id;
    int completion_id;

    EditState *hover_es;            /* edit window that requested the hover */

    EditState *completion_popup;    /* dropdown popup window, NULL when inactive */
    EditState *completion_target_es;/* editor window to insert completion into */
    int        completion_word_start;/* buffer offset of start of partial word */

    const LSPLangConfig *config;

    LSPOpenFile open_files[LSP_MAX_OPEN_FILES];
    int num_open_files;

    char open_on_init[MAX_FILENAME_SIZE]; /* path to open after handshake */
} LSPServer;

extern LSPServer lsp_servers[LSP_MAX_SERVERS];

int        lsp_init(void);
LSPServer *lsp_get_server_for_file(EditState *s);
void       lsp_ensure_did_open(LSPServer *srv, EditState *s);
void       lsp_hover(EditState *s);
void       lsp_complete(EditState *s);

#endif
