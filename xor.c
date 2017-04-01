#include "lab4.h"


pid_t genPid;


void AtExit(int exitCode)
{
	kill(genPid, SIGINT);
	// Parent waits for child's command completion.
	wait(NULL);
	exit(exitCode);
}


int main(void)
{
	int gen_descs[2];
	int cat_descs[2];
	pid_t catPid;
	char *out;
	char path[BUFSIZE];
	char buf[BUFSIZE];
	
	memset(path, '\0', BUFSIZE);
	out = "What file you want to XOR?\n: ";
	write(1, out, strlen(out));
	read(0, path, BUFSIZE);
	path[strlen(path)-1] = '\0';		// Get rid of '\n' at the end

	if (pipe(gen_descs) == -1) {
		perror("Error creating generator pipe");
		AtExit(1);
	}
	genPid = fork();
	switch (genPid) {
	case -1:
		perror("Error forking the generator");
		AtExit(2);
		break;
	case 0:
		memset(buf, '\0', BUFSIZE);
		sprintf(buf, "%d", gen_descs[1]);
		execlp("./gen_m", "./gen_m", buf, NULL);
		break;
	}
	
	if (pipe(cat_descs) == -1) {
		perror("Error creating catalog pipe");
		AtExit(3);
	}
	catPid = fork();
	switch (catPid) {
	case -1:
		perror("Error forking the filewalker");
		AtExit(4);
		break;
	case 0:
		close(cat_descs[0]);
		if (dup2(cat_descs[1], 1) == -1) {
			perror("Error while redirecting cat input");
			AtExit(5);
		}
		close(cat_descs[1]);
		execlp("cat", "cat", path, NULL);
		break;
	default:
		close(cat_descs[1]);
		sleep(1);
		
		char xor[BUFSIZE];
		int readed;
		int genreaded;
		int i;
		int xorfile;
		int xorkey;
	
		memset(buf, '\0', BUFSIZE);
		memset(xor, '\0', BUFSIZE);
		strcat(buf, path);
		strcat(buf, ".crypted");
		xorfile = open(buf, O_RDWR|O_CREAT, 0600);
		if (xorfile == -1) {
			perror("Failed to create xor-file");
			AtExit(6);
		}
		strcat(buf, ".key");
		xorkey = open(buf, O_RDWR|O_CREAT, 0600);
		if (xorkey == -1) {
			perror("Failed to create key-file");
			AtExit(7);
		}
		memset(buf, '\0', BUFSIZE);
		while ((readed = read(cat_descs[0], buf, BUFSIZE)) > 0) {
			genreaded = read(gen_descs[0], xor, readed);
		//	printf("readed = %i, genreaded = %i\n", readed, genreaded);
			if (genreaded != readed) {
				printf("Seems like generator works too slow.");
				printf(" Results will be incorrect. Stopping.\n");
				AtExit(8);
			}
			for (i = 0; i < readed; i++) {
				buf[i] = buf[i] ^ xor[i];
			}
			i = write(xorfile, buf, readed);
			if (i != readed) {
				perror("Error while writing into xor-file");
				AtExit(9);
			}
			i = write(xorkey, xor, readed);
			if (i != readed) {
				perror("Error while writing into key-file");
				AtExit(10);
			}
			memset(buf, '\0', BUFSIZE);
			memset(xor, '\0', BUFSIZE);
		}
		
		close(cat_descs[0]);
		close(xorfile);
		close(xorkey);
		
		AtExit(0);
		
		break;
	}
}
