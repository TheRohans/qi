/* CG: move to header file! */
EditBuffer *new_shell_buffer(const char *name, const char *path, const char **argv, int is_shell);

/* TODO: add state handling to allow colorization of elements longer
 * than one line (eg, multi-line functions and strings)
 */
//static void latex_colorize_line(unsigned int *buf, int len, int *colorize_state_ptr, int state_only);

//static int latex_mode_probe(ModeProbeData *p);

//static int latex_mode_init(EditState *s, ModeSavedData *saved_data);

//static void do_tex_insert_quote(EditState *s);

//static void latex_completion(StringArray *cs, const char *input);

//static void latex_cmd_run(void *opaque, char *cmd);

//static void do_latex(EditState *e, const char *cmd);

int latex_init(void);

