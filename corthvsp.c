#include <stdlib.h>
#include <stdio.h>

void main(int argc, char** argv) {
	if (argc != 3) {
		printf("Error 0x01: improper format\n\
			usage:cvm <source> <target>");
		exit(0x01);
	}
	char* source = argv[1];
	char* target = argv[2];
	*(source+1) = 'a';
	printf("source: %s", source);
}
