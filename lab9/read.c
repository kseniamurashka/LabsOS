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
#include <sys/sem.h>

#define PERMS	0666

const char* sharedMemoryName = "shm_name";
const char* schmAddr = NULL;
const int size = 64;

struct sembuf sem_lock = {0, -1, 0}, sem_open = {0, 1, 0};

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

    int semid = semget(key, 1, PERMS);
	if(semid == -1) {
        int err = errno;
		fprintf(stderr, "Error in semget: %s (%d)\n", strerror(err), err);
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
        semop(semid, &sem_lock, 1);
        time_t ttime;
        ttime = time(NULL);
        struct tm* m_time;
        m_time = localtime(&ttime);
        char str[256];
        sprintf(str,"[From read process]: time: %s pid: %d\n", asctime(m_time), getpid());
        printf("%s\n%s\n", str, schmAddr);
        semop(semid, &sem_open, 1);
        sleep(1);
    }
   
    return 0;
}
