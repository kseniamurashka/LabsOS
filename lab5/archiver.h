#include <sys/types.h>

typedef struct {
    char* name;
    mode_t mode;
    int uid;
    int gid;
    time_t m_time;
    unsigned long size;
    char* content;
} File;

typedef struct {
	char* name;
	unsigned long size;
	unsigned long capacity;
	File** files;	
} Archiver;

Archiver* initArchiver(const char* arch_name);
File* initFile(const char* filename);
File* readFile(char* block);

void printArchStatus(Archiver* arch);

void addFileToArch(Archiver* arch, File* file);
void inputFile(Archiver* arch, const char* filename);
void extractFile(Archiver* arch, const char* filename);

void freeFileMemory(Archiver* arch, File* file);
void freeArchiverMemory(Archiver* arch);

void saveArchiver(Archiver* arch);
