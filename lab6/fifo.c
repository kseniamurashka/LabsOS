#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

int size = 256;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Not enough args: require fifoname\n");
        return 1;
    }
    (void)argc; 
    (void)argv;
    int fifoRes = mkfifo(argv[1], S_IRUSR | S_IWUSR);
    if (fifoRes != 0) {
        int err = errno;
        fprintf(stderr, "Error while processing mkfifo: %s (%d)\n", strerror(err), err);
        return 1;
    }
    int res;
	switch(res = fork()) {
		case -1: {
			int err = errno;
			fprintf(stderr, "Fork error: %s (%d)\n", strerror(err), err);
			break;
		}
		case 0: {
			time_t mytime = time(NULL);
            struct tm *now = localtime(&mytime);
            printf("Child time: %d:%d:%d from process with pid %d\n", now->tm_hour, now->tm_min, now->tm_sec, getpid());

			int fd = open(argv[1], O_RDONLY);
			if(fd == -1) {
				fprintf(stderr, "Error while processing open fifo\n");
				return 1;
			}
			int len = 0;
            char buf[size];
			while((len = read(fd, buf, sizeof(buf))) != 0) {
				write(2, buf, len);
			}
			close(fd);
			break;	
		}
		default: {
			sleep(5);

			time_t mytime = time(NULL);
            struct tm *now = localtime(&mytime);
            char* parent_time = (char*)calloc(1, size);
            sprintf(parent_time, "Parent time: %d:%d:%d from process with pid ", now->tm_hour, now->tm_min, now->tm_sec);
            char* pid = (char*)calloc(1, 16);
			sprintf(pid, "%d\n", getpid());
            strcat(parent_time, pid);

			int fd = open(argv[1], O_WRONLY);
			if(fd == -1) {
				fprintf(stderr, "Error while processing open fifo\n");
				free(parent_time); free(pid);
				return 1;
			}
			write(fd, parent_time, strlen(parent_time) + 1);
			free(parent_time); free(pid); 
            close(fd);
			break;
		}
	}
    unlink(argv[1]);
    return 0;
}
