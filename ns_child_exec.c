#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <signal.h>

#define err_exit(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0)

#define STACK_SIZE 1024 * 1024

static char child_stack[STACK_SIZE];

static void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] cmd [arg...]\n", pname);
    fprintf(stderr, "Options can be:\n");
    fprintf(stderr, "    -i   new IPC namespace\n");
    fprintf(stderr, "    -m   new mount namespace\n");
    fprintf(stderr, "    -n   new network namespace\n");
    fprintf(stderr, "    -p   new PID namespace\n");
    fprintf(stderr, "    -u   new UTS namespace\n");
    fprintf(stderr, "    -U   new user namespace\n");
    fprintf(stderr, "    -v   Display verbose messages\n");
    exit(EXIT_FAILURE);
}

static int child_func(void *args)
{
    char **argv = args;
    execvp(argv[0], &argv[0]);
    err_exit("execvp");
}

int main(int argc, char *argv[])
{
    int opt;
    int flags = 0;
    int verbose = 0;
    pid_t child_pid;

    while ((opt = getopt(argc, argv, "+imnpuUv")) != -1 ) {
        switch (opt) {
        case 'i': flags |= CLONE_NEWIPC;  break;
        case 'm': flags |= CLONE_NEWNS;   break;
        case 'n': flags |= CLONE_NEWNET;  break;
        case 'p': flags |= CLONE_NEWPID;  break;
        case 'u': flags |= CLONE_NEWUTS;  break;
        case 'U': flags |= CLONE_NEWUSER; break;
        case 'v': verbose = 1;            break;
        default: usage(argv[0]);
        }
    }

    child_pid = clone(child_func, child_stack + STACK_SIZE, flags | SIGCHLD, &argv[optind]);
    if (child_pid  == -1)
        err_exit("clone");

    if (verbose)
        printf("%s: PID of child created by clone() is %ld\n", argv[0], (long) child_pid);

    /* Parent falls through to here */

    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        err_exit("waitpid");

    if (verbose)
        printf("%s: terminating\n", argv[0]);

    exit(EXIT_SUCCESS);
}
