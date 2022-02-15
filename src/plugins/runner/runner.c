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

// gofmt -w <filename>
// terraform fmt [options -list=false] [DIR]
// clang-format -i <filename>
//    clang-format -style=llvm -dump-config > .clang-format

static void do_runner(EditState *s, const char *cmd)
{
    const char *argv[4];

    if (cmd == 0) {
        put_status(s, "Run aborted");
        return;
    }

//    argv[0] = "gofmt";
//    argv[1] = "-w";
//    argv[2] = s->b->filename;
//    argv[3] = NULL;
	
    argv[0] = "clang-format";
    argv[1] = "-i";
    argv[2] = s->b->filename;
    argv[3] = NULL;

//	argv[0] = "terraform";
//	argv[1] = "fmt";
//	argv[2] = "-list=false";
//    argv[3] = s->b->filename;
//    argv[4] = NULL;
//	
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
	        put_status(s, "Run failed, couldn't wait for child");
        }
        LOG("%s", "child process finished");
        do_revert_buffer(s);
        put_status(s, "Done");
    } else {
        // Error
        LOG("%s", "Could not fork process");
        put_status(s, "Run failed, couldn't fork process");
    }
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

