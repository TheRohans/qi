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

//    argv[0] = "/bin/sh";
//    argv[1] = "-c";
//    argv[2] = cmd;
//    argv[3] = NULL;

    argv[0] = "gofmt";
    argv[1] = "-w";
    argv[2] = e->b->filename;
    argv[3] = NULL;
    
    getcwd(cwd, sizeof(cwd));

    // get the directory of the open file and change into it
    p = strrchr(e->b->filename, '/');
    if (p == e->b->filename)
        p++;
    len = p - e->b->filename + 1;
    pstrcpy(dir, sizeof(dir), e->b->filename);
    
    LOG("%s", dir)
    chdir(dir);

    pid_t pid = fork();
    if (pid == 0) {
        // child process
        setsid();
        execvp(argv[0], (char *const*)argv);
        exit(1);
    } else if (pid > 0) {
        // parent process
        if(wait(NULL) == -1) {
            LOG("%s", "Could not wait for child process");
        }
        LOG("%s", "child process finished");
        do_revert_buffer(e);
    } else {
        // Error
        LOG("%s", "Could not fork process");
    }

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

