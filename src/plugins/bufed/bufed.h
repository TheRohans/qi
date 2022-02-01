
/* iterate 'func_item' to selected items. If no selected items, then
   use current item */
void string_selection_iterate(StringArray *cs, 
                              int current_index,
                              void (*func_item)(void *, StringItem *),
                              void *opaque);

int bufed_init(void);

