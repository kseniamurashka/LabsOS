#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>

#define NUM_OF_THREADS 10
#define SIZE 10

pthread_mutex_t mut;
int nums[SIZE];

int cur_idx = 0;
void* Read(void* arg) {
    (void)arg;
    while (cur_idx < SIZE) {
        sleep(1); 
        /* заблокировали мьютекс, поток ждет пока мьютекс освободится */
        pthread_mutex_lock(&mut);

        printf("Reading...\n");
        sleep(1); 
        printf(" cur_tid: %i\tcur_state: [ ", (pid_t)gettid()); // gettid() как getpid(), но для тредов (у них pid одинаковый)
        for (int i = 0; i < SIZE; i++) printf ("%d ", nums[i]);
        printf("]\n");
        sleep(1);

        /* разблокировали мьютекс, теперь он может быть использован другими потоками */
        pthread_mutex_unlock(&mut);
    } 

    /* программа ждет завершения работы потока */
    pthread_exit(NULL);
}

void* Write(void* arg) {
    (void)arg;
    for (int i = 0; i < SIZE; i++) {
        sleep(1);
        pthread_mutex_lock(&mut);

        printf("\nWriting process...\n\n");
        cur_idx++;
	nums[i] = i + 1;

	pthread_mutex_unlock(&mut);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_mutex_init(&mut, NULL);

    pthread_t readThreads[NUM_OF_THREADS];
    for (int i = 0; i < NUM_OF_THREADS; i++) {
        int status1 = pthread_create(&readThreads[i], NULL, Read, NULL);
        if (status1 != 0) {
            int err = errno;
            fprintf(stderr, "Error in pthread_create for read[%d]: %s (%d)\n", i, strerror(err), err);
            return 1;
        }
    }

    pthread_t writeThread;
    int status2 = pthread_create(&writeThread, NULL, Write, NULL);
    if (status2 != 0) {
        int err = errno;
        fprintf(stderr, "Error in pthread_create for write: %s (%d)\n", strerror(err), err);
        return 1;
    }
    for (int i = 0; i < NUM_OF_THREADS; i++) {
	void* status = NULL;
        /* блокирует вызывающий поток пока readThreads[i] работает */
	int status3 = pthread_join(readThreads[i], &status);
	if (status3 != 0) {
		int err = errno;
		fprintf(stderr, "Error in pthread_join: %s(%d)\n", strerror(err), err);		
        return 1;
	}
}

    pthread_join(writeThread, NULL);
    pthread_mutex_destroy(&mut);
    return 0;
}
