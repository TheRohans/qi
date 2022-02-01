/**
 * Runner mode for Qi.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "qe.h"


static void do_runner(EditState *e, const char *cmd)
{
    // struct latex_function *func = (struct latex_function *)opaque;
    char cwd[MAX_FILENAME_SIZE];
    char dir[MAX_FILENAME_SIZE];
    const char *argv[4];
    char *p;
    int len;

    if (cmd == 0) {
        // put_status(func->es, "aborted");
        return;
    }

    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = cmd;
    argv[3] = NULL;

    getcwd(cwd, sizeof(cwd));

    /* get the directory of the open file and change into it
     */
    p = strrchr(e->b->filename, '/');
    if (p == e->b->filename)
        p++;
    len = p - e->b->filename + 1;
    pstrcpy(dir, sizeof(dir), e->b->filename);
    chdir(dir);

    /* if (func->output_to_buffer) {
        // if the buffer already exists, kill it 
        EditBuffer *b = eb_find("*LaTeX output*");
        if (b) {
            // XXX: e should not become invalid
            b->modified = 0;
            do_kill_buffer(func->es, "*LaTeX output*");
        }

        // create new buffer
        // TODO: fix the shell plugin to get this to work, but
        // I don't think this should rely on the other plugin being loaded
        // b = new_shell_buffer("*LaTeX output*", "/bin/sh", argv, 0);
        // if (b) {
        //     // XXX: try to split window if necessary
        //     switch_to_buffer(func->es, b);
        // }
    } else { */
        int pid = fork();
        if (pid == 0) {
            // child process
            setsid();
            execv("/bin/sh", (char *const*)argv);
            exit(1);
        }
    // }
    chdir(cwd);
}

/* specific runner commands */
static CmdDef runner_commands[] = {
    CMD_( KEY_CTRLX(KEY_CTRL('r')), KEY_NONE, "runner", do_runner,
          "s{Runner command: }|runner|")
    CMD_DEF_END,
};


int runner_init(void)
{
    /* commands and default keys */
    qe_register_cmd_table(runner_commands, "runner");
    
    return 0;
}

