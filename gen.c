#include "lab4.h"

int pipeDesc;

void AtInterruption(int signal)
{
	close(pipeDesc);
	printf("Stopping generator...\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	char randnum[10];
	int i;
	
	signal(SIGINT, AtInterruption);
	srand(time(NULL));
	if (sscanf(argv[1], "%d", &pipeDesc) != 1) {
		perror("sscanf error in gen.c");
		exit(1);
	}
	while (1) {
		for (i = 0; i < 10; i++)
			randnum[i] = (char)(rand() % 256);
		write(pipeDesc, randnum, 10);
	}
}
