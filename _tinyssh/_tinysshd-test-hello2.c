/*
20160225
Jan Mojzis
Public domain.
*/

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "log.h"
#include "packet.h"
#include "global.h"
#include "str.h"
#include "writeall.h"

static void cleanup(void) {

    global_purge();
}

static void die_fatal(const char *trouble, const char *d, const char *fn) {

    cleanup();

    if (d) {
        if (fn) log_f5(trouble, " ", d, "/", fn);
        else log_f3(trouble, " ", d);
    }
    else {
        log_f1(trouble);
    }
    _exit(111);
}


int main(int argc, char **argv) {

    pid_t pid;
    int tochild[2] = { -1, -1 };
    int fromchild[2] = { -1, -1 };
    const char *message;
    long long messagelen;

    if (argc < 2) _exit(111);
    if (!argv[0]) _exit(111);
    if (!argv[1]) _exit(111);
    if (!argv[2]) _exit(111);
    ++argv;
    message = *argv;
    messagelen = str_len(message);
    ++argv;

    if (pipe(tochild) == -1) _exit(111);
    if (pipe(fromchild) == -1) _exit(111);
    pid = fork();
    if (pid == -1) _exit(111);

    if (pid == 0) {
        close(tochild[1]);
        close(fromchild[0]);
        if (dup2(tochild[0], 0) == -1) _exit(111);
        if (dup2(fromchild[1], 1) == -1) _exit(111);
        execvp(*argv, argv);
        _exit(111);
    }
    close(tochild[0]);
    close(fromchild[1]);

    close(0); if (dup2(fromchild[0], 0) == -1) _exit(111);
    close(1); if (dup2(tochild[1], 1) == -1) _exit(111);

    signal(SIGPIPE, SIG_IGN);

    global_init();

    if (!packet_hello_receive()) die_fatal("unable to receive hello-string", 0, 0);
    if (messagelen) {
        if (writeall(1, message, messagelen) == -1) die_fatal("unable to write hello-string", 0, 0);
        if (writeall(1, "\r\n", 2) == -1) die_fatal("unable to write hello-string", 0, 0);
    }

    _exit(111);
}
