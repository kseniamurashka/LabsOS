#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

void atExitHandler() {
    printf("[atexit] I'm atExitHandler for process %d\n", getpid());
}
void signalHandler(int sig) {
    printf("[signal] Signal %d recieved\n", sig);
}
void sigactionHandler(int sig, siginfo_t *siginfo, void *arg) {
    printf("[sigaction] Signal %d recieved from process %u \n", sig, siginfo->si_pid);
    printf("[sigaction] Signumber is %d; an errno value is %d\n", siginfo->si_signo, siginfo->si_errno);
    printf("[sigaction] Sending process ID is %d; sending uid is %d\n", siginfo->si_pid, siginfo->si_uid);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv; 
    //printf("I am process %i\n", getpid());

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &sigactionHandler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, NULL);

    atexit(atExitHandler);
    signal(SIGINT, signalHandler);

    int res = 0;
    switch (res = fork()) {
        case -1:
            int err = errno;
            fprintf(stderr, "Fork error : %s (%d)\n", strerror(err), err);
            break;
        case 0:
            printf("[CHILD] I'm child of %d, my pid is %d\n", getppid(), getpid());
            break;
        default:
            int ch_res;
            wait(&ch_res);
            printf("[PARENT] I'm parent of %d, my pid id %d, my parent pid is %d\n", res, getpid(), getppid());
            printf("[PARENT] Child exit code %d\n", WEXITSTATUS(ch_res));
            break;
    }
    sleep(7);
    return 0;
}
