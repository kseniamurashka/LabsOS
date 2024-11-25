#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define PERMS	0666

const char* sharedMemoryName = "shm_name";
const char* schmAddr = NULL;
const int size = 64;

int main(int argc, char** argv) {
    (void)argc, (void)argv;

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

    time_t ttime;
    ttime = time(NULL);
    struct tm* m_time;
    m_time = localtime(&ttime);
    char str[256];
    sprintf(str,"[Receiving process]: time: %s pid: %d\n", asctime(m_time), getpid());
    printf("%s\n", str);
    sleep(5);

    if (schmAddr != NULL) {
        printf("%s\n", schmAddr);
        int res = 0;
        if ((res = shmdt(schmAddr)) < 0) {
            int err = errno;
            fprintf(stderr, "Error in shmdt: %s (%d)\n", strerror(err), err);
            return 1;
        }
    }
    sleep(5);

    return 0;
}
