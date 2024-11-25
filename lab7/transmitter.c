#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>


#define PERMS	0666

char* sharedMemoryName = "shm_name";
char* schmAddr = NULL;
const int size = 64;

void heandler(int signal) {
    printf ("[STOP SIGNAL] %d\n", signal);
    if (schmAddr != NULL) {
        /* отстыковывает сегмент разделяемой памяти, находящийся по адресу schmAddr */
        int res = shmdt(schmAddr);
        if (res < 0) {
            int err = errno;
            fprintf(stderr, "Error in shmdt: %s (%d)\n", strerror(err), err);
            exit(1);
        }
    }
    exit(0);
}


int main(int argc, char** argv) {
    (void)argc, (void)argv;

    /* создать файл, если такого еще нет*/
    int fd = open(sharedMemoryName, O_CREAT|O_EXCL, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        fprintf(stderr, "Receiving process is already running\n");
        close(fd);
        return 1;    
    }
    close (fd);

    /* получить ключ для системного вызова */
    key_t key = ftok(sharedMemoryName, 1);
    if (key == (key_t)-1) {
        int err = errno;
        fprintf(stderr, "Error in ftok: %s (%d)\n", strerror(err), err);
        return -1;
    }

    /* создание сегмента разделяемой памяти */
    int shmid = shmget(key, size, PERMS | IPC_CREAT);
    if (shmid < 0) {
        int err = errno;
        fprintf(stderr, "Error in shmget: %s (%d)\n", strerror(err), err);
        return 1;
    }

    /* получение адреса сегмента */
    schmAddr = shmat(shmid, NULL, 0);
    if (schmAddr == (void*)-1) {
        int err = errno;
        fprintf(stderr, "Error in shmat: %s (%d)\n", strerror(err), err);
        return 1;
    }

    signal(SIGINT, heandler);
    signal(SIGTERM, heandler);

    while (1) {
        sleep(1);

        time_t ttime;
        ttime = time(NULL);
        struct tm* m_time;
        m_time = localtime(&ttime);
        char str[256];
        sprintf(str,"[Trancsimitting process] time: %s pid: %d\n", asctime(m_time), getpid());
        strcpy(schmAddr, str);
    }

    return 0;
}
