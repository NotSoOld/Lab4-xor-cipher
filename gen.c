#include "lab4.h"


int main(int argc, char *argv[])
{
	char randnum[1];
	int pipe;
	
	srand(time(NULL));
	sscanf(argv[1], "%d", &pipe);
	while(1)
	{
		randnum[0] = (char)(rand() % 256);
		write(pipe, randnum, 1);
	}
	exit(0);
}
