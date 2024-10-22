#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

#include <time.h>
#include <string.h>
#include <ctype.h>

#define RESET   "\033[0m"
#define BLUE     "\033[1;34m"//Директория – выделение синим;
#define GREEN  "\033[1;32m"//Исполняемый файл – выделение зелёным;
#define TURQUOISE   "\033[1;36m"//Ссылка – выделение бирюзовым.
#define WHITE   "\033[37m"//Обычный файл – без выделения;

struct line {
    char is_dir;
    mode_t mode;
    unsigned int nlink;
    char* uid;
    char* gid;
    unsigned long size;
    char* time;
    char* name;
    char* linkpointer;
};

char* toLowReg(char* str) {
    char* low = malloc(strlen(str));
    for (int i = 0; i < strlen(str); i++) {
        low[i] = tolower(str[i]);
    }
    return low;
}

char* const substr(char* s, size_t pos, size_t count)
{
   static char buf[BUFSIZ];
   memset(buf, '\0', BUFSIZ);
   return strncpy(buf, s + pos, count);
}

int isExectFile(mode_t mode) {
    int statchmod = mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    char oct[9];
    sprintf(oct, "%o", statchmod);

    for (int i = 0; i < 9; i++) {
        if (oct[i] == '1' || oct[i] == '3' || oct[i] == '5' || oct[i] == '7' ) return 1;
    }
    return 0;
}

void printNames(char* name, mode_t mode, char* del) {
    if (S_ISLNK(mode & S_IFMT)) {
        printf("%s'%s'%s%s", TURQUOISE, name, RESET, del);//ссылка
    } else if (S_ISDIR(mode & S_IFMT)){
        printf("%s%s%s%s", BLUE, name, RESET, del);//директория
    } else if (isExectFile(mode) == 1) {
        printf("%s%s%s%s", GREEN, name, RESET, del);//исполянемый файл
    } else {
        printf("%s%s%s%s", WHITE, name, RESET, del);//обычный файл
    }
}

void printAccessRights(mode_t mode) {
    int statchmod = mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    char oct[9];
    sprintf(oct, "%o", statchmod);

    for (int i = 0; i < 9; i++) {
        if (oct[i] == '0') printf("---");
        else if (oct[i] == '1') printf("--x");
        else if (oct[i] == '2') printf("-w-");
        else if (oct[i] == '3') printf("-wx");
        else if (oct[i] == '4') printf("r--");
        else if (oct[i] == '5') printf("r-x");
        else if (oct[i] == '6') printf("rw-");
        else if (oct[i] == '7') printf("rwx");
    }
}


size_t totalSize(struct line data[], int num) {
    size_t total = 0;
    for (int i = 0; i < num; i++) {
        if (strcmp(data[i].name, ".") == 0 || strcmp(data[i].name, "..") == 0) continue;
        total += ((data[i].size / 1024) + (data[i].size % 1024 != 0));
    }
    return total;
}

int myLsFunction(const char* path_name, int fl_a, struct line data[]) {
    DIR* cur_dir = opendir(path_name);
    struct dirent *dir;
    struct stat st;
    
    int pos = 0;
    while ((dir = readdir(cur_dir)) != NULL) {
        char wrk[256];
        strcpy(wrk, path_name);
        strcat(wrk, "/");
        strcat(wrk, dir->d_name);
        int answ = lstat(wrk, &st);
        if (answ != 0) {
            perror("lstat");
            exit(-1);
        }

        struct line cur_line = {};
        if (fl_a == 0 && dir->d_name[0] == '.') continue;

        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            cur_line.is_dir = 'd';
        } else if (S_ISLNK(st.st_mode & S_IFMT)) {
            cur_line.is_dir = 'l';
        } else {
            cur_line.is_dir = '-';
        }

        cur_line.mode = st.st_mode;

        cur_line.nlink = st.st_nlink;

        struct passwd *pwd;
        pwd = getpwuid(st.st_uid);
        if(pwd == NULL) {
            cur_line.uid = malloc(10);
            sprintf(cur_line.uid, "%d", st.st_uid);
        } else {
            char* uid = pwd->pw_name;
            cur_line.uid = malloc(strlen(uid));
            strcpy(cur_line.uid, uid);
        }

        struct passwd *pwd_;
        pwd_ = getpwuid(st.st_gid);
        if(pwd_ == NULL) {
            cur_line.gid = malloc(10);
            sprintf(cur_line.gid, "%d", st.st_gid);
        } else {
            char* gid = pwd_->pw_name;
            cur_line.gid = malloc(strlen(gid));
            strcpy(cur_line.gid, gid);
        }

        cur_line.size = st.st_size;

        time_t time = st.st_mtime;
        char* subs = substr(ctime(&time), 4, 12);
        cur_line.time = malloc(strlen(subs));
        strcpy(cur_line.time, subs);

        char* cur_name = dir->d_name;
        cur_line.name = malloc(strlen(cur_name));
        strcpy(cur_line.name, cur_name);

        if (S_ISLNK(st.st_mode & S_IFMT)) {
            char linkname[256];
            
            ssize_t r = readlink(wrk, linkname, st.st_size);
            if (r == -1) {
                perror("readlink");
                exit(1);
            }
            
            linkname[r] = 0;
            cur_line.linkpointer = malloc(strlen(linkname));
            strcpy(cur_line.linkpointer, linkname);
        }

        data[pos++] = cur_line;
    }
    closedir(cur_dir);
    return pos;
}

void strSort(struct line data[], int num) {
    for (int i = 1; i < num; i++) {
        for (int j = 0; j < num - i; j++) {
            struct line tmp;
            char* name1 = toLowReg(data[j].name);
            char* name2 =  toLowReg(data[j + 1].name);
            if (strcmp(name1, name2) > 0) {
                tmp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = tmp;
            }
            free(name1); free(name2);
        }
    }
}

void printStructure(struct line data[], int num) {
    size_t curLenOfAllNames = 0;
    for (int i = 0; i < num; i++) {
        if (curLenOfAllNames + 1 + strlen(data[i].name) > 128) {
            printf("\n");
            curLenOfAllNames = 0;
        }
        printNames(data[i].name, data[i].mode, "  ");   
        curLenOfAllNames += 1 + strlen(data[i].name);
    }
    printf ("\n");
}

void printStructureList(struct line data[], int num) {
    for (int i = 0; i < num; i++) {
        printf("%c", data[i].is_dir);
        printAccessRights(data[i].mode);
        printf(" %4d", data[i].nlink);
        printf(" %s", data[i].uid);
        printf(" %s", data[i].gid);
        printf("%8lu", data[i].size);
        printf(" %s ", substr(data[i].time, 0, 3));//month
        printf("%s ", substr(data[i].time, 4, 2));//day
        printf("%s ", substr(data[i].time, 7, 5));//time
        if (data[i].is_dir == 'l') {
             printNames(data[i].name, data[i].mode, "");   
            printf (" -> %s\n", data[i].linkpointer);
        } else  printNames(data[i].name, data[i].mode, "\n");  
    }
}

int main(int argc, char** argv) {
    if (argc < 1) return 0;

    char flags[2];
    int i = 0;
    char c;
    while ((c = getopt(argc, argv, "al")) != -1) {
        flags[i++] = c;
    }

    int flag_a = 0, flag_l = 0;
    if (flags[0] == 'a' || flags[1] == 'a') flag_a = 1;
    if (flags[0] == 'l' || flags[1] == 'l') flag_l = 1;

    char* path_name;
    if (optind >= argc) {
        path_name = ".";
    } else path_name = argv[optind];

    DIR* cur_dir = opendir(path_name);
    if (!cur_dir) {
        fprintf(stderr, "can't open current dirrectory");
        exit(1);
    }

    struct line data[65535];
    if (argc == 1) {
        int num = myLsFunction(path_name, 0, data);
        strSort(data, num);
        printStructure(data, num);
        closedir(cur_dir);
        return 0;
    }

    
    
    int num = myLsFunction(path_name, flag_a, data);
    strSort(data, num);
    if (flag_l == 1) {
        printf("total %ld\n", totalSize(data, num));
        printStructureList(data, num);
    } else {
        printStructure(data, num);
    }

    closedir(cur_dir);
    return 0;
}
