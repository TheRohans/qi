//static void build_bufed_list(EditState *s);
//static EditBuffer *bufed_get_buffer(EditState *s);
//static void bufed_select(EditState *s, int temp);
/* iterate 'func_item' to selected items. If no selected items, then
   use current item */
void string_selection_iterate(StringArray *cs, 
                              int current_index,
                              void (*func_item)(void *, StringItem *),
                              void *opaque);

//static void bufed_kill_item(void *opaque, StringItem *item);
//static void bufed_kill_buffer(EditState *s);

/* show a list of buffers */
//static void do_list_buffers(EditState *s);
//
//static void bufed_clear_modified(EditState *s);
//static void bufed_toggle_read_only(EditState *s);
//
//static void bufed_refresh(EditState *s, int toggle);
//static void bufed_display_hook(EditState *s);
//
//static int bufed_mode_init(EditState *s, ModeSavedData *saved_data);
//
//static void bufed_mode_close(EditState *s);

int bufed_init(void);

