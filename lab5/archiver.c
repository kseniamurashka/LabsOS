#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <pwd.h>
#include <sys/stat.h>

#include "Archiver.h"

const unsigned MAX_BLOCK_SIZE = 512;
const char sep = ';';

File* readFile(char* block) {
    File* file = (File*)calloc(1, sizeof(File)); 
 
    char* curFilePos = strchr(block, sep);
    int curSize = strlen(block) - strlen(curFilePos);

    file->name = (char*)calloc(1, curSize + 1);
	strncpy(file->name, block, curSize);
	sscanf(curFilePos + 1, "%u", &file->mode); 
	curFilePos = strchr(curFilePos + 1, sep);
	sscanf(curFilePos + 1, "%d", &file->uid);
	curFilePos = strchr(curFilePos + 1, sep);
	sscanf(curFilePos + 1, "%d", &file->gid);
	curFilePos = strchr(curFilePos + 1, sep);
	sscanf(curFilePos + 1, "%ld", &file->m_time);
	curFilePos = strchr(curFilePos + 1, sep);
	sscanf(curFilePos + 1, "%ld", &file->size);
	file->content = (char*)calloc(1, file->size + 1);
    return file;
}

void addFileToArch(Archiver* arch, File* file) {
    if (arch->files == NULL) {
		arch->files = (File**)calloc(2, sizeof(File*));
		arch->capacity = 2;
	}

	if (arch->size + 1 >= arch->capacity) {
		arch->capacity *= 2;
		arch->files = (File**)realloc(arch->files, arch->capacity * sizeof(File*));
	}

	arch->files[arch->size] = file;
    arch->size++;
}

Archiver* initArchiver(const char* arch_name) {
    Archiver* arch = (Archiver*)calloc(1, sizeof(Archiver));
    arch->name=(char*)calloc(1, strlen(arch_name) + 1);
    strcpy(arch->name, arch_name);
    arch->size = 0;
    arch->capacity = 0;
    arch->files = NULL;

    int fd = open(arch->name, O_RDWR);
    if (fd == -1) {
        printf("Archiver %s was created\n", arch_name);
        close(fd);
        return arch;
    }

    int archSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, 0);
    char* block = (char*)calloc(1, MAX_BLOCK_SIZE + 1);
    while(lseek(fd, 0, SEEK_CUR) != archSize) {
        read(fd, block, MAX_BLOCK_SIZE);
        File* file = readFile(block);
        read(fd, file->content, file->size);
        addFileToArch(arch, file);
    }
    free(block);
    close(fd);
    return arch;
}

void printArchStatus(Archiver* arch) {
    printf("Archiver name: %s\nNumber of files: %ld\n--------------\n", arch->name, arch->size);
    for (int i = 0; i < arch->size; i++) {
        File* file = arch->files[i];
        printf("Filename: %s\nmode: %u, uid: %d, gid: %d, size: %ld\n", file->name, file->mode, file->uid, file->gid, file->size);
        printf("%s\n", file->content);
    }
}

File* initFile(const char* filename) {
    int fd = open(filename, O_RDONLY);
	if(fd == -1) {
		fprintf(stderr, "Error in opening file\n");
		return NULL;
	}

    File* file = (File*)calloc(1, sizeof(File));

    struct stat st;
	int answ = stat(filename, &st);
	if(answ == -1) {
		fprintf(stderr, "Error in stat\n");
		return NULL;
	}
    file->size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, 0);

    file->name = (char*)calloc(1, sizeof(filename) + 1);
    strcpy(file->name, filename);
    file->mode = st.st_mode;
	file->uid = st.st_uid;
	file->gid = st.st_gid;
	file->m_time = st.st_mtime;
    file->content = (char*)calloc(1, file->size + 1);
    read(fd, file->content, file->size);

    close(fd);
    return file;
}

void saveArchiver(Archiver* arch) {
    int fd = open(arch->name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Error in saving archiver\n");
    }

    for (int i = 0; i < arch->size; i++) {
        char* fileBlock = (char*)calloc(1, MAX_BLOCK_SIZE + 1);
		File* file = arch->files[i];
		int infoLineSize = sprintf(fileBlock, "%s;%u;%d;%d;%ld;%ld", file->name, file->mode, file->uid, file->gid, file->m_time, file->size);	
		for (int i = infoLineSize; i < MAX_BLOCK_SIZE; i++) fileBlock[i] = '?';
		write(fd, fileBlock, MAX_BLOCK_SIZE);
		write(fd, file->content, file->size);	
		free(fileBlock);

    }
    close(fd);
}

void inputFile(Archiver* arch, const char* filename) {
    File* file = initFile(filename);
    if (file == NULL) {
        return;
    }
    addFileToArch(arch, file);
    saveArchiver(arch);
    printf("Success input file: %s\n", filename);
}

void extractFile(Archiver* arch, const char* filename) {
    for (int i = 0; i < arch->size; i++) {
        if (strcmp(arch->files[i]->name, filename) == 0) {
            File* file = arch->files[i];
            int fd = creat(filename, file->mode);
            if (fd == -1) {
                fprintf(stderr, "Error in creating extracted file %s\n", filename);
                close(fd);
                return;
            }
            fchmod(fd, file->mode);
            write(fd, file->content, file->size);
            close(fd);
            struct utimbuf new_time;
            new_time.modtime = file->m_time;
            utime(filename, &new_time);

            freeFileMemory(arch, arch->files[i]);
            if (i != arch->size - 1) {
                arch->files[i] = arch->files[arch->size - 1];
            }
            arch->size--;
            saveArchiver(arch);
            printf("Success extract file: %s\n", filename);
            return;
        }
    }
    printf("Can't extract file %s: no such file in archive %s\n", filename, arch->name);
}

void freeFileMemory(Archiver* arch, File* file) {
    if (file->name != NULL) free(file->name);
	if (file->content != NULL) free(file->content);
	free(file);
}

void freeArchiverMemory(Archiver* arch) {
    for (int i = 0; i < arch->size; i++) {
        if (arch->files[i] != NULL) freeFileMemory(arch, arch->files[i]);
    }
    if (arch->files != NULL) free(arch->files);
    if (arch->name != NULL) free(arch->name);
    free(arch); 
}
