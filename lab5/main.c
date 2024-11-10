#include <stdio.h>
#include "archiver.h"
#include <string.h>
#include <stdlib.h>

void printHelp() {
    printf("{arch_name} -i(--input)   {filename}\tadd file in archive\n");
    printf("{arch_name} -e(--extract) {filename}\tdelete file from archive\n");
    printf("{arch_name} -s(--stat)\t                print status of archive\n");
    printf("            -h(--help)\t                print help for working with archiver\n");
}


void errorWhat(char* error_text){
    fprintf(stderr, error_text);
    printf ("Command not found, try ./archiver -h(--help) to get more information\n");
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        errorWhat("not enough args\n");
        return -1;
    }
    
    if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			printHelp();
			return 0;
		}
		else {
			errorWhat("Unknown flag\n");
			return -1;
		}
	}

    Archiver* arch = initArchiver(argv[1]);
    if (argc == 3) {
		if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--stat") == 0) {
			printArchStatus(arch);
		}
		else {
			errorWhat("Unknown flag\n");
		}
	}
    else if (argc == 4) {
		if (strcmp(argv[2], "-i") == 0 || strcmp(argv[2], "--input") == 0) {
            inputFile(arch, argv[3]);
		}
		else if(strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--extract") == 0) {
			extractFile(arch, argv[3]);
		}
		else {
			errorWhat("Unknown flag\n");
		}
	}
    freeArchiverMemory(arch);
    return 0;
}
