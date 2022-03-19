/*
 * Typescript / Javascript mode for Qi.
 */
#include "qe.h"
#include "../clang/clang.h"

static const char ts_keywords[] =
    "|async|await|break|case|catch|class|const|continue|debugger|default|"
    "default|delete|do|else|enum|export|extends|false|finally|"
    "for|function|if|import|in|instanceof|new|null|return|super|"
    "switch|this|throw|true|try|typeof|var|while|with|"
    // strict mode
    "as|implements|interface|let|package|private|protected|"
    "public|static|yield|"
    // context
    "constructor|delare|get|module|require|set|symbol|type|"
    "from|of|namespace|";

static const char ts_types[] =
    "|any|boolean|string|number|null|unknown|void|undefined|"
    "Boolean|Null|Undefined|Number|BigInt|String|Symbol|Object|Infinity|NaN|"
    "Function|Promise|Array|Date|"
    // ECMAScript 2015
    "Int8Array|Uint8Array|Uint8ClampedArray|Int16Array|Uint16Array|Int32Array|"
    "Uint32Array|Float32Array|Float64Array|BigInt64Array|BigUint64Array|"
    "Set|Map|WeakSet|";

static int get_ts_keyword(char *buf, int buf_size, unsigned int **pp) {
  unsigned int *p, c;
  char *q;

  p = *pp;
  c = *p;
  q = buf;
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') ||
      (c == '$')) {
    do {
      if ((q - buf) < buf_size - 1)
        *q++ = c;
      p++;
      c = *p;
    } while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') ||
             (c == '$') || (c >= '0' && c <= '9'));
  }
  *q = '\0';
  *pp = p;
  return q - buf;
}

// colorization states
enum {
  TS_COMMENT = 1,
  TS_STRING,
  TS_STRING_Q,
};

void ts_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
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
  case TS_COMMENT:
    goto parse_comment;
  case TS_STRING:
  case TS_STRING_Q:
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
    case '/':
      p++;
      if (*p == '*') {
        // normal comment
        p++;
        state = TS_COMMENT;
      parse_comment:
        while (*p != '\n') {
          if (p[0] == '*' && p[1] == '/') {
            p += 2;
            state = 0;
            break;
          } else {
            p++;
          }
        }
        set_color(p_start, p - p_start, QE_STYLE_COMMENT);
      } else if (*p == '/') {
        // line comment
        while (*p != '\n')
          p++;
        set_color(p_start, p - p_start, QE_STYLE_COMMENT);
      }
      break;
    case '#':
      p++;
      set_color(p_start, p - p_start, QE_STYLE_PREPROCESS);
      break;
    case '`':
      state = TS_STRING_Q;
      goto string;
    case '\'':
      state = TS_STRING_Q;
      goto string;
    case '\"':
      // strings/chars
      state = TS_STRING;
    string:
      p++;
    parse_string:
      while (*p != '\n') {
        if (*p == '\\') {
          p++;
          if (*p == '\n')
            break;
          p++;
        } else if ((*p == '\'' && state == TS_STRING_Q) ||
                   (*p == '`' && state == TS_STRING_Q) ||
                   (*p == '\"' && state == TS_STRING)) {
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

        get_ts_keyword(kbuf, sizeof(kbuf), &p);
        p1 = p;
        while (*p == ' ' || *p == '\t')
          p++;
        if (strfind(ts_keywords, kbuf, 0)) {
          set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
        } else if (strfind(ts_types, kbuf, 0)) {
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

static int ts_mode_probe(ModeProbeData *p) {
  const char *r;
  r = extension(p->filename);
  if (*r) {
    if (strfind("|js|ts|jsx|tsx|json|", r + 1, 1))
      return 100;
  }
  return 0;
}

int ts_mode_init(EditState *s, ModeSavedData *saved_data) {
  int ret;

  ret = text_mode_init(s, saved_data);
  if (ret)
    return ret;
  set_colorize_func(s, ts_colorize_line);
  return ret;
}

// void do_tslangfmt(EditState *s)
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

static CmdDef ts_commands[] = {
    CMD0(KEY_META(';'), KEY_NONE, "c-comment", do_c_comment)
    CMD0(KEY_CTRLX(';'), KEY_NONE, "c-comment-region", do_c_comment_region)

    // CMDV('{', KEY_NONE, "c-electric-obrace", do_c_electric, '{', "*v")
    // CMDV( '(', KEY_NONE, "c-electric-paren", do_c_electric, '(', "*v")
    CMDV(KEY_RET, KEY_NONE, "c-electric-newline", do_c_electric, '\n', "*v")
    // CMD0( KEY_CTRLX('y'), KEY_NONE, "c-fmt", do_clangfmt)
    CMD_DEF_END,
};

static ModeDef ts_mode;
int ts_init(void) {
  // mode is almost like the text mode, so we copy and patch it
  memcpy(&ts_mode, &text_mode, sizeof(ModeDef));

  ts_mode.name = "Typescript";
  ts_mode.mode_probe = ts_mode_probe;
  ts_mode.mode_init = ts_mode_init;

  qe_register_mode(&ts_mode);

  qe_register_cmd_table(ts_commands, "Typescript");

  return 0;
}
