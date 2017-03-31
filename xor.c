#include "lab4.h"


int main(void)
{
	int gen_descs[2];
	int cat_descs[2];
	pid_t genPid;
	pid_t catPid;
	char *out;
	char *path;
	
	out = "What file you want to XOR?\n: ";
	write(1, out, strlen(out));
	read(0, path, BUFSIZE);
	//path[strlen(path)-1] = '\0';
	

	if (pipe(gen_descs) == -1) {
		perror("Error creating generator pipe");
		exit(3);
	}
	genPid = fork();
	switch (genPid) {
	case -1:
		perror("Error forking the generator");
		exit(1);
		break;
	case 0:
		
		execl("gen", "gen", gen_descs[1], NULL);
		break;
	default:
		
		break;
	}
	
	if (pipe(cat_descs) == -1) {
		perror("Error creating catalog pipe");
		exit(4);
	}
	catPid = fork();
	switch (catPid) {
	case -1:
		perror("Error forking the filewalker");
		exit(1);
		break;
	case 0:
		close(0);
		if (dup(cat_descs[0]) == -1) {
			perror("Error while redirecting cat input");
			exit(5);
		}
		close(cat_descs[0]);
		close(cat_descs[1]);
		execl("cat", "cat", path, NULL);
		break;
	default:
		close(cat_descs[0]);
		
		close(cat_descs[1]);
		break;
	}
	exit(0);
}
