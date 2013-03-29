#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include "qe.h"
#include "qstack.h"
#include "quick-open.h"

QuickOpenEntry* newEntry(struct dirent* entry, char* parent, char* fullpath) {
	struct QuickOpenEntry* ent_copy = malloc(sizeof(struct QuickOpenEntry));
	ent_copy->entry = malloc(sizeof(struct dirent));
	ent_copy->parent = malloc(sizeof(char) * strlen(parent)+1);
	ent_copy->fullpath = malloc(sizeof(char) * strlen(fullpath)+1);
	
	memcpy(ent_copy->entry, entry, sizeof(struct dirent));
	memcpy(ent_copy->parent, parent, strlen(parent)+1);
	memcpy(ent_copy->fullpath, fullpath, strlen(fullpath)+1);
	
	return ent_copy;
}

void freeEntry(QuickOpenEntry* entry) {
	free(entry->entry);
	free(entry->parent);
	free(entry->fullpath);
	free(entry);
}

void recursive_dir(char* dir, char* search, Stack* S)
{
	DIR* dp;
	struct dirent* entry;
	struct stat statbuf;
	if((dp = opendir(dir)) == NULL) {
		fprintf(stderr, "cannot open directory: %s\n", dir);
		return;
	}
	
	chdir(dir);
	while((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);
		
		if(S_ISDIR(statbuf.st_mode)) {
			/* Found a directory, but ignore . and .. */
			if(strcmp(".", entry->d_name) == 0 ||
				strcmp("..", entry->d_name) == 0)
					continue;
			
			recursive_dir(entry->d_name, search, S);
		} else {
			//example of simple string compare
			if( strstr(entry->d_name, search) != NULL ) {
				
				//really? C is kinda dangerous :-D
				char actualpath [MAX_QI_PATH+1];
				realpath(dir, actualpath);
				
				////////////////////////////////////////////////////
				struct QuickOpenEntry* ent_copy = newEntry(entry, dir, actualpath);
				
				if(!stack_full(S)) stack_push(S, (void *)ent_copy);
				////////////////////////////////////////////////////
				
				//printf("%p\n", entry);
				//printf("(%s)\n", dir);
				//printf("--> %s/%s\n", actualpath, entry->d_name);
				//printf("%s [%x]\n", entry->d_name, entry->d_ino);
			}
		}
	}
	chdir("..");
	closedir(dp);
}

//////////////////////////////////////////////////
//int main()
//{
//
//	Stack S;
//	stack_init(&S);
//	int i;
//	
//	printf("\n");
//	recursive_dir("../../", "charset", &S);
//	
//	printf("\n");
//	dump_stack(&S);
//	printf("\n");
//	
//	int top = S.top;
//	struct QuickOpenEntry* entry;
//	for (i=0; i<top; i++)
//	{
//		entry = (struct QuickOpenEntry*)stack_pop(&S);
//		
//		printf("%p\n", entry);
//		printf("%s (%s) [%x]\n", entry->entry->d_name, entry->parent, entry->entry->d_ino); 
//		printf("%s/%s\n", entry->fullpath, entry->entry->d_name); 
//		
//		freeEntry(entry);
//	}
//	
//	printf("\n");
//	dump_stack(&S);
//	printf("\n");
//	
//	return 0;
//}
////////////////////////////////////////////////////

EditBuffer* new_quickopen_buffer(int* show_ptr)
{
    EditBuffer *b;

    *show_ptr = 0;
    b = eb_find("*QuickOpen*");
    if (b) {
        eb_delete(b, 0, b->total_size);
    } else {
        b = eb_new("*QuickOpen*", BF_SYSTEM);
        *show_ptr = 1;
    } 
    return b;
}

void do_quick_open(EditState *s, const char *filename)
{
	Stack S;
	stack_init(&S);
	int i;
	
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		recursive_dir(cwd, filename, &S);
	
	    EditBuffer *b;
	    int show;

	    b = new_quickopen_buffer(&show);
	    if (!b)
	        return;

	    //eb_printf(b, 
	    //          "Qi help for help - Press C-g (q) to quit:\n"
	    //          "\n"
	    //          "C-x ? (F1)                Show this help\n"
	    //          "M-x describe-bindings     Display table of all key bindings\n"
	    //          "M-x describe-key-briefly  Describe key briefly\n"
	    //          );
	
		struct QuickOpenEntry* entry;
		int top = S.top;
		for (i=0; i<top; i++)
		{
			entry = (struct QuickOpenEntry*)stack_pop(&S);
			eb_printf(b, "  %s/%s\n", entry->fullpath, entry->entry->d_name);
			freeEntry(entry);
		}
	    b->modified = 0;
	    //b->flags |= BF_READONLY;
		b->flags |= BF_DIRED;
	
	    if (show) {
	        show_popup(b);
	    }
		chdir(cwd);
	}
	
	stack_free(&S);
}

ModeDef quick_open_mode;

//void do_quick_open(EditState *s)
//{
//    //DiredState *hs;
//    QEmacsState *qs = s->qe_state;
//	EditBuffer *b, *b0;
//    EditState *e, *e1;
//    int width, index, i;
//    //char filename[MAX_FILENAME_SIZE], *p;
//    //
//    ///* Should take directory argument with optional switches,
//    // * find dired window if exists,
//    // * else create one and do this.
//    // * recursive listing and multi directory patterns.
//    // */
//    //
//    ///* remember current buffer for target positioning, because
//    // * s may be destroyed by  insert_window_left
//    // */
//    //b0 = s->b;
//    //
//    //b = eb_new("*dired*", BF_READONLY | BF_SYSTEM);
//    //
//    ///* set the filename to the directory of the current file */
//    //pstrcpy(filename, sizeof(filename), s->b->filename);
//    //p = strrchr(filename, '/');
//    //if (p)
//    //    *p = '\0';
//    //canonize_absolute_path(filename, sizeof(filename), filename);
//    //set_filename(b, filename);
//    
//    width = qs->width / 4;
//    e = insert_window_left(b, width, WF_MODELINE);
//    do_set_mode(e, &quick_open_mode, NULL);
//    //hs = e->mode_data;
//
//    //e1 = find_window(e, KEY_RIGHT);
//    //if (e1)
//    //    b0 = e1->b;
//    //
//    //index = 0;
//    ///* CG: target file should be an argument to this command */
//    //for (i = 0; i < hs->items.nb_items; i++) {
//    //    if (get_dired_filename(e, filename, sizeof(filename), i)
//    //    &&  !strcmp(filename, b0->filename)) {
//    //        index = i;
//    //        break;
//    //    }
//    //}
//    //e->offset = eb_goto_pos(e->b, index + DIRED_HEADER, 0);
//    //
//    ///* modify active window */
//    qs->active_window = e;
//}


static CmdDef quick_open_commands[] = {
    CMD_DEF_END,
};

static CmdDef quick_open_global_commands[] = {
    //CMD0( KEY_CTRLX(KEY_CTRL('o')), KEY_NONE, "quickopen", do_quick_open)
	CMD_( KEY_CTRLX(KEY_CTRL('o')), KEY_NONE, "quickopen", do_quick_open, "s{Find file: }|file|")
    CMD_DEF_END,
};

int quick_open_init(void)
{
    /* inherit from list mode */
    /* CG: assuming list_mode already initialized ? */
    memcpy(&quick_open_mode, &list_mode, sizeof(ModeDef));
    quick_open_mode.name = "quickopen";
    //dired_mode.instance_size = sizeof(DiredState);
    //dired_mode.mode_probe = dired_mode_probe;
    //dired_mode.mode_init = dired_mode_init;
    //dired_mode.mode_close = dired_mode_close;
    //dired_mode.display_hook = dired_display_hook;
    
    /* first register mode */
    qe_register_mode(&quick_open_mode);

    qe_register_cmd_table(quick_open_commands, "quickopen");
    qe_register_cmd_table(quick_open_global_commands, NULL);

    return 0;
}

