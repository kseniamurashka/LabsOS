#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>

int size = 256; 

int main(int argc, char** argv) {
    (void)argc; 
    (void)argv;

    int pipedesc[2];
    int pipeRes = pipe(pipedesc);
    if (pipeRes != 0) {
        int err = errno;
        fprintf(stderr, "Error while processing pipe: %s (%d)\n", strerror(err), err);
        return 1;
    }

    int res = 0;
    switch((res = fork())) {
        case -1: {
            int err = errno;
            fprintf(stderr, "Fork error: %s (%d)\n",strerror(err), err);
            break;
        } case 0:{
            time_t mytime = time(NULL);
            struct tm *now = localtime(&mytime);
            printf("Child time: %d:%d:%d from process with pid %d\n", now->tm_hour, now->tm_min, now->tm_sec, getpid());
    
			close(pipedesc[1]);//закрыли на запись
            int len = 0;
            char buf[size];
			while((len = read(pipedesc[0], buf, sizeof(buf))) != 0) {
				write(2, buf, len);
			}
			close(pipedesc[0]);
            break;
        } default: {
            sleep(5);

            time_t mytime = time(NULL);
            struct tm *now = localtime(&mytime);
            char* parent_time = (char*)calloc(1, size);
            sprintf(parent_time, "Parent time: %d:%d:%d from process with pid ", now->tm_hour, now->tm_min, now->tm_sec);
            char* pid = (char*)calloc(1, 16);
			sprintf(pid, "%d\n", getpid());
            strcat(parent_time, pid);
			
			close(pipedesc[0]);
			write(pipedesc[1], parent_time, strlen(parent_time) + 1);

			free(parent_time);
			free(pid);
			close(pipedesc[1]);
            break;
        }     
    }
    return 0;
}
