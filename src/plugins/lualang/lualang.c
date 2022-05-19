/*
 * Lua mode for Qi.
 */
#include "qe.h"
#include "../clang/clang.h"

static const char lua_keywords[] = "|and|break|do|else|elseif|"
                                   "|end|false|for|function|if|"
                                   "|in|not|or|print|"
                                   "|repeat|return|then|true|until|while|";

static const char lua_types[] = "|local|nil|";

static int get_lua_keyword(char *buf, int buf_size, unsigned int **pp) {
  unsigned int *p, c;
  char *q;

  p = *pp;
  c = *p;
  q = buf;
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
    do {
      if ((q - buf) < buf_size - 1)
        *q++ = c;
      p++;
      c = *p;
    } while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') ||
             (c == '.') || (c == ':') || (c >= '0' && c <= '9'));
  }
  *q = '\0';
  *pp = p;
  return q - buf;
}

// colorization states
enum {
  LUA_COMMENT = 1,
  LUA_STRING,
  LUA_STRING_Q,
};

void lua_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
                       int state_only) {
  int c, state, type_decl;
  unsigned int *p, *p_start, *p1;
  char kbuf[32];

  state = *colorize_state_ptr;
  p = buf;
  p_start = p;
  type_decl = 0;
  c = 0;

  // if already in a state, go directly in the code parsing it
  switch (state) {
  case LUA_COMMENT:
  	goto parse_comment;
  case LUA_STRING:
  case LUA_STRING_Q:
    goto parse_string;
  default:
    break;
  }

  for (;;) {
    p_start = p;
    c = *p;
    switch (c) {
    case '\n':
      goto the_end;
    case '-':
      p++;
      if (*p == '-') {
        p++;
        // Single line comment for sure,
        // Maybe multi line?
        if (*p == '[' && p[1] == '[') {
          p += 2;
          // Ok multi line
          state = LUA_COMMENT;
parse_comment:
          while (*p != '\n') {
            if (p[0] == '-' && p[1] == '-' && p[2] == ']' && p[3] == ']') {
              p += 4;
              state = 0;
              break;
            } else {
              p++;
            }
          }
          set_color(p_start, p - p_start, QE_STYLE_COMMENT);
        } else {
          // single line comment
          while (*p != '\n')
            p++;
          set_color(p_start, p - p_start, QE_STYLE_COMMENT);
        }
      }
      break;
    /* case '#':
      p++;
      set_color(p_start, p - p_start, QE_STYLE_PREPROCESS);
      break; */
    /* case '`':
      state = TS_STRING_Q;
      goto string; */
    case '\'':
      state = LUA_STRING_Q;
      goto string;
    case '\"':
      // strings/chars
      state = LUA_STRING;
    string:
      p++;
    parse_string:
      while (*p != '\n') {
        if (*p == '\\') {
          p++;
          if (*p == '\n')
            break;
          p++;
        } else if ((*p == '\'' && state == LUA_STRING_Q) ||
                   // (*p == '`' && state == TS_STRING_Q) ||
                   (*p == '\"' && state == LUA_STRING)) {
          p++;
          state = 0;
          break;
        } else {
          p++;
        }
      }
      set_color(p_start, p - p_start, QE_STYLE_STRING);
      break;
    case '=':
      p++;
      // exit type declaration
      type_decl = 0;
      break;
    default:
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
        get_lua_keyword(kbuf, sizeof(kbuf), &p);
        p1 = p;
        while (*p == ' ' || *p == '\t')
          p++;
        if (strfind(lua_keywords, kbuf, 0)) {
          set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
        } else if (strfind(lua_types, kbuf, 0)) {
          // if not cast, assume type declaration
          if (*p != ')') {
            type_decl = 1;
          }
          set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
        } else {
          // assume typedef if starting at first column
          if (p_start == buf)
            type_decl = 1;

          if (type_decl) {
            if (*p == '(') {
              // function definition case
              set_color(p_start, p1 - p_start, QE_STYLE_FUNCTION);
              type_decl = 1;
            } else if (p_start == buf) {
              // assume type if first column
              set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
            } else {
              set_color(p_start, p1 - p_start, QE_STYLE_VARIABLE);
            }
          }
        }
      } else {
        p++;
      }
      break;
    }
  }
the_end:
  *colorize_state_ptr = state;
}

#define MAX_BUF_SIZE 512
#define MAX_STACK_SIZE 64

enum {
  INDENT_NORM,
  INDENT_FIND_EQ,
};

static int lua_mode_probe(ModeProbeData *p) {
  const char *r;
  r = extension(p->filename);
  if (*r) {
    if (strfind("|lua|", r + 1, 1))
      return 100;
  }
  return 0;
}

int lua_mode_init(EditState *s, ModeSavedData *saved_data) {
  int ret;

  ret = text_mode_init(s, saved_data);
  if (ret)
    return ret;
  set_colorize_func(s, lua_colorize_line);
  return ret;
}

// void do_lualangfmt(EditState *s)
//{
//	const char *argv[4];
//
//	// XXX: configure option? Scripting option?
//    argv[0] = "clang-format";
//    argv[1] = "-i";
//    argv[2] = s->b->filename;
//    argv[3] = NULL;
//
//	run_system_cmd(s, argv);
//}

static CmdDef lua_commands[] = {
    // CMD0(KEY_META(';'), KEY_NONE, "c-comment", do_c_comment)
    // CMD0(KEY_CTRLX(';'), KEY_NONE, "c-comment-region", do_c_comment_region)
    CMDV(KEY_RET, KEY_NONE, "c-electric-newline", do_c_electric, '\n', "*v")
        CMD_DEF_END,
};

static ModeDef lua_mode;
int lua_init(void) {
  // mode is almost like the text mode, so we copy and patch it
  memcpy(&lua_mode, &text_mode, sizeof(ModeDef));

  lua_mode.name = "Lua";
  lua_mode.mode_probe = lua_mode_probe;
  lua_mode.mode_init = lua_mode_init;

  qe_register_mode(&lua_mode);

  qe_register_cmd_table(lua_commands, "Lua");

  return 0;
}
