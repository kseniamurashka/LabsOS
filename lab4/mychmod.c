#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int makeMode(char* rights) {
    int mode = 0;
    int digit = rights[0] - '0';
    digit = digit << 6;
    mode = mode | digit;
    digit = rights[1] - '0';
    digit = digit << 3;
    mode = mode | digit;
    digit = rights[2] - '0';
    mode = mode | digit;

    return mode;
}

int rightsSettings(char* path, char* settings) {
    struct stat st;
    int answ = lstat(path, &st);
    if (answ != 0) {
        perror("lstat");
        exit(-1);
    }
    mode_t prev_mode = st.st_mode;
    int statchmod = prev_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    char prev_rights[9];
    sprintf(prev_rights, "%o", statchmod);

    printf("current right: %s\n", prev_rights);

    int user = 0, group = 0, others = 0;
    int op = 1;
    int r = 0, w = 0, x = 0;
    for (int i = 0; i < strlen(settings); i++) {
        if (settings[i] == 'u') user = 1;
        if (settings[i] == 'g') group = 1;
        if (settings[i] == 'o') others = 1;

        if (settings[i] == '+') op = 1;
        if (settings[i] == '-') op = -1;

        if (settings[i] == 'r') r = 0b100;
        if (settings[i] == 'w') w = 0b010;
        if (settings[i] == 'x') x = 0b001;
    }

    int perm = r | w | x;
    int mode = 0;
    if (user == 1) {
        mode = mode | (perm << 6);
    }
    if (group == 1) {
        mode = mode | (perm << 3);
    }
    if (others == 1) {
        mode = mode | perm;
    }

    if (op == 1) {
        mode = mode | prev_mode;
    } else if (op == -1) {
        mode = (~mode) & prev_mode;
    }
    printf("new mode: %d\n", mode);
    return mode;
}

void setNewRights(char* path, char* rights) {
    int ch = atoi(rights);
    int mode;
    if (ch != 0 && strlen(rights) == 3) {
        mode = makeMode(rights);
    } else {
        mode = rightsSettings(path, rights);
    }
    chmod(path, mode);
}

int main(int argc, char** argv) 
{
    if (argc < 3) {
        fprintf(stderr, "not anough args\n");
        return 1;
    }

    char* rights = argv[optind++];
    char* filepath = argv[optind];

    int fd = open(filepath, O_RDWR | O_CREAT);
    if (fd == -1) {
        perror("Error of open file\n");
        return -1;
    }
    
    setNewRights(filepath, rights);
    
    printf("%s %s\n", rights, filepath);

    return 0;
}
