/*
 * Markdown mode for Qi
 * Copyright (c) 2013 Rob Rohan
 * Based on c-mode by Fabrice Bellard
 */
#include "qe.h"

#define MAX_BUF_SIZE 512

static ModeDef markdown_mode;

enum ParseState {
  IN_LINK = 1,
  OUT_LINK,
  IN_CODE_BLOCK,
  OUT_CODE_BLOCK,
};

static void markdown_colorize_line(unsigned int *buf, int len,
                                   int *colorize_state_ptr, int state_only) {
  int c, state;
  unsigned int *p, *p_start;

  state = *colorize_state_ptr;
  p = buf;
  p_start = p;

  for (;;) {
    p_start = p;
    c = *p;

    if (state == IN_CODE_BLOCK && (c != '`' && c != '\n')) {
      // if we are in a code block, and the char isn't possibly
      // going to get us out, just colour the rest of the line.
      goto in_code_block;
    }

    switch (c) {
    case '\n': {
      goto the_end;
    } break;

    case '`': {
      p++;
      int tick_count = 1;

      while (*p == '`') {
        p++;
        tick_count++;
      }

      if (tick_count == 3 && state != IN_CODE_BLOCK) {
        state = IN_CODE_BLOCK;
      } else if (tick_count == 3 && state == IN_CODE_BLOCK) {
        state = OUT_CODE_BLOCK;
      }

      if (state == IN_CODE_BLOCK) {
        // if we are in a triple ` eat all the rest of the
        // chars up until the end of the line
        while (*p != '\n')
          p++;
      } else if (tick_count == 1) {
        // inline code block
        state = OUT_CODE_BLOCK;
        while (*p != '\n') {
          p++;
          if (*p == '`') {
            p++;
            break;
          }
        }
      }

      set_color(p_start, p - p_start, QE_STYLE_COMMENT);
    } break;

    case '(': {
      p++;
      if (state == IN_LINK) {
        while (*p != '\n') {
          p++;
          if (*p == ')') {
            p++;
            break;
          }
        }
        state = OUT_LINK;
        set_color(p_start, p - p_start, QE_STYLE_COMMENT);
      }
    } break;

    case '#':
      p++;
      while (*p != '\n') {
        p++;
      }
      set_color(p_start, p - p_start, QE_STYLE_STRING);
      break;
    case '-':
      p++;
      short dash = 0;
      while (*p != '\n') {
        p++;
        dash++;
        if (*p != '-')
          break;
      }
      if (dash == 2) {
        set_color(p_start, p - p_start, QE_STYLE_HIGHLIGHT);
      }
      break;
    case '[':
      p++;
      while (*p != '\n') {
        p++;
        if (*p == ']') {
          p++;
          state = IN_LINK;
          break;
        }
      }
      set_color(p_start, p - p_start, QE_STYLE_TYPE);
      break;
    case '_': {
      p++;
      int two = 0;
      if (*p == '_')
        two = 1;
      while (*p != '\n') {
        p++;
        if (*p == '_') {
          p++;
          if (two)
            p++;
          break;
        }
      }
      set_color(p_start, p - p_start, QE_STYLE_KEYWORD);
      break;
    }
    case '*': {
      p++;
      int two = 0;
      if (*p == '*')
        two = 1;

      while (*p != '\n') {
        p++;
        if (*p == '*') {
          p++;
          if (two)
            p++;
          break;
        }
      }
      set_color(p_start, p - p_start, QE_STYLE_KEYWORD);
      break;
    }
    default:
      if (state == IN_CODE_BLOCK) {
      in_code_block:
        // if we are in a triple ` eat all the rest of the
        // chars up until the end of the line
        while (*p != '\n')
          p++;
        set_color(p_start, p - p_start, QE_STYLE_COMMENT);
      } else {
        p++;
        break;
      }
    }
  }

the_end:
  *colorize_state_ptr = state;
}

static int markdown_mode_probe(ModeProbeData *p) {
  const char *r;

  // currently, only use the file extension
  r = extension(p->filename);
  if (*r) {
    if (strfind("|md|markdown|mdown|mkd|mdtext|mdtxt|mkdn|", r + 1, 1))
      return 100;
  }
  return 0;
}

static int markdown_mode_init(EditState *s, ModeSavedData *saved_data) {
  int ret;
  ret = text_mode_init(s, saved_data);
  if (ret)
    return ret;
  set_colorize_func(s, markdown_colorize_line);
  return ret;
}

// specific Markdown commands
static CmdDef markdown_commands[] = {
    CMD_DEF_END,
};

int markdown_init(void) {
  // Markdown mode is almost like the text mode, so we copy and patch it
  memcpy(&markdown_mode, &text_mode, sizeof(ModeDef));
  markdown_mode.name = "Markdown";
  markdown_mode.mode_probe = markdown_mode_probe;
  markdown_mode.mode_init = markdown_mode_init;

  qe_register_mode(&markdown_mode);
  qe_register_cmd_table(markdown_commands, "Markdown");
  // register_completion("markdown", markdown_completion);

  return 0;
}
