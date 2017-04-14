#include "lab4.h"

void AtInterruption(int signal)
{
	// The singal will be sent by xor program.
	printf("Stopping generator...\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	char randnum[16];
	int i;
	
	signal(SIGINT, AtInterruption);
	srand(time(NULL));
	
	// Just continiously write random bytes into the pipe.
	while (1) {
		for (i = 0; i < 16; i++)
			randnum[i] = (char)(rand() % 256);
		write(1, randnum, 16);
	}
}
