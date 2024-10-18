#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

    int user = 0, group = 0, others = 0, all = 0;
    int op = 0;
    int r = 0, w = 0, x = 0;

    int settings_step = 0, error = 0;
    for (int i = 0; i < strlen(settings); i++) {
        if (settings_step == 0) {
            switch (settings[i])
            {
                case 'u': { user = 1; break;}
                case 'g': { group = 1; break;}
                case 'o': { others = 1; break;}
                default: {
                    if (user + group + others == 0) all = 1;
                    settings_step = 1;
                    break;
                }
            }
        }
        if (settings_step == 1) {
            switch (settings[i])
            {
                case '+': { op = 1; break;}
                case '-': { op = -1; break;}
                default: { settings_step = 2; break;}
            }
        }
        if (settings_step == 2) {
            switch (settings[i])
            {
                case 'r': { r = 4; break;}
                case 'w': { w = 2; break;}
                case 'x': { x = 1; break;}
                default: { error = 1; break;}
            }
        }
    }

    if (!op || error) return -1;

    int perm = r | w | x;
    int mode = 0;
    if (user || all) mode = mode | (perm << 6);
    if (group || all) mode = mode | (perm << 3);
    if (others || all) mode = mode | perm;

    if (op == 1) mode = mode | prev_mode;
    else if (op == -1) mode = (~mode) & prev_mode;
    
    return mode;
}

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

void setNewRights(char* path, char* rights) {
    int ch = atoi(rights);
    int mode;
    if (strlen(rights) == 3) {
        mode = makeMode(rights);
    } else {
        mode = rightsSettings(path, rights);
        if (mode == -1) {
            printf("chmod: invalid mode: %s\n", rights);
            return;
        }
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
    
    setNewRights(filepath, rights);

    return 0;
}
