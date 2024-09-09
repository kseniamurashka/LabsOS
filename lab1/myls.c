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


#define RESET   "\033[0m"
#define BLUE     "\033[1;34m"//Директория – выделение синим;
#define GREEN  "\033[1;32m"//Исполняемый файл – выделение зелёным;
#define TURQUOISE   "\033[1;36m"//Ссылка – выделение бирюзовым.
#define WHITE   "\033[37m"//Обычный файл – без выделения;

const char* const substr(char* s, size_t pos, size_t count)
{
   static char buf[BUFSIZ];
   memset(buf, '\0', BUFSIZ);
   return strncpy(buf, s + pos, count);
}

void printNames(struct dirent *dir, struct stat st, char* del) {
    if (S_ISDIR(st.st_mode)) {
        printf("%s%s%s%s", BLUE, dir->d_name, RESET, del);//директория
    } else if (S_ISBLK(st.st_mode)){
        printf("%s%s%s%s", TURQUOISE, dir->d_name, RESET, del);//ссылка
    } else if (strstr(dir->d_name, ".exe")) {
            printf("%s%s%s%s", GREEN, dir->d_name, RESET, del);//исполняемый файл
    } else {
        printf("%s%s%s%s", WHITE, dir->d_name, RESET, del);//обычный файл
    }
}

size_t totalSize(DIR* cur_dir, const char* path_name) {
    struct dirent *dir;
    struct stat st;

    size_t total = 0;
    while ((dir = readdir(cur_dir)) != NULL) {
        char wrk[256];
        strcpy(wrk, path_name);
        strcat(wrk, "/");
        strcat(wrk, dir->d_name);
        int answ = stat(wrk, &st);
        if (answ != 0) {
            fprintf(stderr, "Error in calling stat()");
            exit(-1);
        }

        total += st.st_size;
    }
    return total;
}

void myLsFunction(DIR* c_dir, const char* path_name, int fl_a, int fl_l) {
    struct dirent *dir;
    struct stat st;
    if (fl_l == 1) {
        printf ("total %ld\n", totalSize(c_dir, path_name));
    }

    DIR* cur_dir = opendir(path_name);
    while ((dir = readdir(cur_dir)) != NULL) {
        char wrk[256];
        strcpy(wrk, path_name);
        strcat(wrk, "/");
        strcat(wrk, dir->d_name);
        int answ = stat(wrk, &st);
        if (answ != 0) {
            fprintf(stderr, "Error in calling stat()");
            exit(-1);
        }

        if (fl_l == 1) {
            if (fl_a == 0 && dir->d_name[0] == '.') continue;
            char* del = "\n";
            if (S_ISDIR(st.st_mode)) {//1
                printf("d ");
            } else printf("-r ");

            printf("%9u ", st.st_mode);//2

            printf("%3ld ", st.st_nlink);//3

            struct passwd *pwd;
            pwd = getpwuid(st.st_uid);
            if(pwd == NULL) {
                 perror("getpwuid");
            } else {
                printf("%s ", pwd->pw_name);//4.1
            }

            //struct passwd *pwd;
            pwd = getpwuid(st.st_gid);
            if(pwd == NULL) {
                 perror("getpwuid");
            } else {
                printf("%s ", pwd->pw_name);//4.2
            }

            printf("%12lu ", st.st_size);//5

            time_t time = st.st_mtime;
            printf("%s ", substr(ctime(&time), 4, 12));//6
            
            printNames(dir, st, del);

        } else {
            char* del = "  ";
            if (fl_a == 0){
                if (dir->d_name[0] != '.') printNames(dir, st, del);
            } else printNames(dir, st, del);
        }
    }
}

int main(int argc, char** argv) {
    //if (argc == 1) return 0;

    char* path_name = ".";
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '.') path_name = argv[i];
    }

    DIR* cur_dir = opendir(path_name);
    if (!cur_dir) {
        fprintf(stderr, "can't open current dirrectory");
        exit(1);
    }

    char c;
    if (argc == 1) {
        myLsFunction(cur_dir, path_name, 0, 0);
        closedir(cur_dir);
        printf("\n");
        return 0;
    }

    char flags[2];
    int i = 0;
    while ((c = getopt(argc, argv, "al")) != -1) {
        flags[i++] = c;
    }

    int flag_a = 0, flag_l = 0;
    if (flags[0] == 'a' || flags[1] == 'a') flag_a = 1;
    if (flags[0] == 'l' || flags[1] == 'l') flag_l = 1;
    myLsFunction(cur_dir, path_name, flag_a, flag_l);

    closedir(cur_dir);
    printf("\n");
    return 0;
}
