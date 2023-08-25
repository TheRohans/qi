/*
 * SQL mode for Qi.
 */
#include "qe.h"
#include "../clang/clang.h"

static const char sql_keywords[] =
    "|do|elseif|handler|if|iterate|leave|loop|repeat|resignal|signal|until|"
    "while|abs|absent|acos|all|allocate|alter|and|any|any_value|are|array|"
    "array_agg|array_max_cardinality|as|asensitive|asin|asymmetric|at|atan|"
    "atomic|authorization|avg|begin|begin_frame|begin_partition|between|bigint|"
    "binary|blob|boolean|both|btrim|by|call|called|cardinality|cascaded|case|"
    "cast|ceil|ceiling|char|character|character_length|char_length|check|"
    "classifier|clob|close|coalesce|collate|collect|column|commit|condition|"
    "connect|constraint|contains|convert|copy|corr|corresponding|cos|cosh|"
    "count|covar_pop|covar_samp|create|cross|cube|cume_dist|current|current_"
    "catalog|current_date|current_default_transform_group|current_path|current_"
    "role|current_row|current_schema|current_time|current_timestamp|current_"
    "transform_group_for_type|current_user|cursor|cycle|date|day|deallocate|"
    "dec|decfloat|decimal|declare|default|define|delete|dense_rank|deref|"
    "describe|deterministic|disconnect|distinct|double|drop|dynamic|each"
    "|element|else|empty|end|end-exec|end_frame|end_partition|equals|escape|"
    "every|except|exec|execute|exists|exp|external|extract|false|fetch|filter|"
    "first_value|float|floor|for|foreign|frame_row|free|from|full|function|"
    "fusion|get|global|grant|greatest|group|grouping|groups|having|hold|hour|"
    "identity|in|indicator|initial|inner|inout|insensitive|insert|int|integer|"
    "intersect|intersection|interval|into|is|join|json|json_array|json_"
    "arrayagg|json_exists|json_object|json_objectagg|json_query|json_scalar|"
    "json_serialize|json_table|json_table_primitive|json_value|lag|language|"
    "large|last_value|lateral|lead|leading|least|left|like|like_regex|listagg|"
    "ln|local|localtime|localtimestamp|log|log10|lower|lpad|ltrim|match|"
    "matches|match_number|match_recognize|max|member|merge|method|min|minute|"
    "mod|modifies|module|month|multiset|national|natural|nchar|nclob|new|no|"
    "none|normalize|not|nth_value|ntile|null|nullif|numeric|occurrences_regex|"
    "octet_length|of|offset|old|omit|on|one|only|open|or|order|out|outer|over|"
    "overlaps|overlay"
    "|parameter|partition|pattern|per|percent|percentile_cont|percentile_disc|"
    "percent_rank|period|portion|position|position_regex|power|precedes|"
    "precision|prepare|primary|procedure|ptf|range|rank|reads|real|recursive|"
    "ref|references|referencing|regr_avgx|regr_avgy|regr_count|regr_intercept|"
    "regr_r2|regr_slope|regr_sxx|regr_sxy|regr_syy|release|result|return|"
    "returns|revoke|right|rollback|rollup|row|rows|row_number|rpad|running|"
    "savepoint|scope|scroll|search|second|seek|select|sensitive|session_user|"
    "set|show|similar|sin|sinh|skip|smallint|some|specific|specifictype|sql|"
    "sqlexception|sqlstate|sqlwarning|sqrt|start|static|stddev_pop|stddev_samp|"
    "submultiset|subset|substring|substring_regex|succeeds|sum|symmetric|"
    "system|system_time|system_user|table|tablesample|tan|tanh|then|time|"
    "timestamp|timezone_hour|timezone_minute|to|trailing|translate|translate_"
    "regex|translation|treat|trigger|trim|trim_array|true|truncate|uescape|"
    "union|unique|unknown|unnest|update|upper|user|using|value|values|value_of|"
    "varbinary|varchar|varying"
    "|var_pop|var_samp|versioning|when|whenever|where|width_bucket|window|with|"
    "within|without|year|";

static const char sql_types[] = ""; // |local|nil|";

static int get_sql_keyword(char *buf, int buf_size, unsigned int **pp) {
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
    } while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= '0' && c <= '9'));
  }
  *q = '\0';
  *pp = p;
  return q - buf;
}

// colorization states
enum {
  SQL_COMMENT = 1,
  SQL_STRING,
  SQL_STRING_Q,
};

void sql_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr,
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
  case SQL_COMMENT:
    goto parse_comment;
  case SQL_STRING:
  case SQL_STRING_Q:
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
        p++;
        state = SQL_COMMENT;
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
      }
      break;
    case '-':
      p++;
      if (*p == '-') {
        // single line comment
        while (*p != '\n')
          p++;
        set_color(p_start, p - p_start, QE_STYLE_COMMENT);
      }
      break;
    case '\'':
      state = SQL_STRING_Q;
      goto string;
    case '\"':
      // strings/chars
      state = SQL_STRING;
    string:
      p++;
    parse_string:
      while (*p != '\n') {
        if (*p == '\\') {
          p++;
          if (*p == '\n')
            break;
          p++;
        } else if ((*p == '\'' && state == SQL_STRING_Q) ||
                   // (*p == '`' && state == TS_STRING_Q) ||
                   (*p == '\"' && state == SQL_STRING)) {
          p++;
          state = 0;
          break;
        } else {
          p++;
        }
      }
      set_color(p_start, p - p_start, QE_STYLE_STRING);
      break;
    /* case '=':
      p++;
      // exit type declaration
      type_decl = 0;
      break; */
    default:
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
        get_sql_keyword(kbuf, sizeof(kbuf), &p);
        p1 = p;
        while (*p == ' ' || *p == '\t')
          p++;
        if (strfind(sql_keywords, kbuf, 1)) {
          set_color(p_start, p1 - p_start, QE_STYLE_KEYWORD);
        } else if (strfind(sql_types, kbuf, 1)) {
          // if not cast, assume type declaration
          if (*p != ')') {
            type_decl = 1;
          }
          set_color(p_start, p1 - p_start, QE_STYLE_TYPE);
        } /* else {
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
        } */
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

static int sql_mode_probe(ModeProbeData *p) {
  const char *r;
  r = extension(p->filename);
  if (*r) {
    if (strfind("|sql|", r + 1, 1))
      return 100;
  }
  return 0;
}

int sql_mode_init(EditState *s, ModeSavedData *saved_data) {
  int ret;

  ret = text_mode_init(s, saved_data);
  if (ret)
    return ret;
  set_colorize_func(s, sql_colorize_line);
  return ret;
}

static CmdDef sql_commands[] = {
    // CMD0(KEY_META(';'), KEY_NONE, "c-comment", do_c_comment)
    // CMD0(KEY_CTRLX(';'), KEY_NONE, "c-comment-region", do_c_comment_region)
    CMDV(KEY_RET, KEY_NONE, "c-electric-newline", do_c_electric, '\n', "*v")
        CMD_DEF_END,
};

static ModeDef sql_mode;
int sql_init(void) {
  // mode is almost like the text mode, so we copy and patch it
  memcpy(&sql_mode, &text_mode, sizeof(ModeDef));

  sql_mode.name = "SQL";
  sql_mode.mode_probe = sql_mode_probe;
  sql_mode.mode_init = sql_mode_init;

  qe_register_mode(&sql_mode);

  qe_register_cmd_table(sql_commands, "SQL");

  return 0;
}
