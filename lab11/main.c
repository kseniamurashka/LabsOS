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

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int cur_idx = 0;
int nums[SIZE];

void* Read(void* arg) {
    (void)arg;
    char* result_str = (char*)calloc(1, 256);
    while (cur_idx < SIZE) {
        /* заблокировали мьютекс, поток ждет пока мьютекс освободится */
        sleep(1);
        pthread_rwlock_rdlock(&rwlock);
        sprintf(result_str, "Reading...\n%li\tcur_state: [ ", pthread_self()); 
        char* cur_state_str = (char*)calloc(1, 256);
        for (int i = 0; i < SIZE; i++) {
            sprintf (cur_state_str, "%d ", nums[i]);
            strcat(result_str, cur_state_str);
        }
        strcat(result_str, "]\n");
        printf("%s", result_str);
        free(cur_state_str);
        /* разблокировали мьютекс, теперь он может быть использован другими потоками */
        pthread_rwlock_unlock(&rwlock);
    } 
    free(result_str);
    pthread_exit(NULL);
}

void* Write(void* arg) {
    (void)arg;
    for (int i = 0; i < SIZE; i++) {
        pthread_rwlock_wrlock(&rwlock);
        sleep(1);
        printf("\nWriting process...\n\n");
        cur_idx++;
		nums[i] = i + 1;
		pthread_rwlock_unlock(&rwlock);
    }
    pthread_exit(NULL);
}

int main() {
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
    
	pthread_join(writeThread, NULL);
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
    return 0;
}
