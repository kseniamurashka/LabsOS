#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>//getopt

#include <sys/types.h>
#include <string.h>

int main(int argc, char** argv) {
    char* path_name = ".";
    if (optind >= argc) {
		char* line = NULL;
		size_t len = 0;
		while (getline(&line, &len, stdin) != -1) {
			printf("%s", line);
		}
		if (line != NULL) {
			free(line);
		}
		return 0;
	}
	else {
		path_name = argv[optind];
	}

    int i = 0;
    char c;
    int flag_n = 0, flag_b = 0, flag_E = 0;
    while ((c = getopt(argc, argv, "nbE")) != -1) {
        if (c == 'n') flag_n = 1;
        if (c == 'b') flag_b = 1;
        if (c == 'E') flag_E = 1;
        else printf("Unknown flag\n");
        i++;
    }

    FILE* file = fopen(path_name, "r");
    if (file == NULL) {
        printf("Cant't open file\n");
        return 0;
    }

    char* line;
    size_t len = 0;
    ssize_t read;
    size_t number = 1;
    while ((read = getline(&line, &len, file)) != -1) { 
        if (flag_n == 1 || flag_b == 1) {
            if ((flag_b == 1 && read > 2) || flag_b == 0) {
                printf ("     %ld  ", number++);
            }
        }
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
        char e;
        if (flag_E == 1) e = '$';
        printf("%s%c\n", line, e);
    }
    fclose(file);
    return 0;
}
