#include "lab4.h"

pid_t catkeyPid;
pid_t catfilePid;

void AtExit(int exitCode)
{
	kill(catkeyPid, SIGINT);
	kill(catfilePid, SIGINT);
	// Parent waits for child's command completion.
	wait(NULL);
	exit(exitCode);
}

int main(void)
{
	int catkey_descs[2];
	int catfile_descs[2];
	char *out;
	char path[BUFSIZE];
	char buf[BUFSIZE];
	
	memset(path, '\0', BUFSIZE);
	out = "What file you want to unXOR?\n: ";
	write(1, out, strlen(out));
	read(0, path, BUFSIZE);
	path[strlen(path)-1] = '\0';		// Get rid of '\n' at the end

	if (pipe(catkey_descs) == -1) {
		perror("Error creating cat key-file pipe");
		AtExit(1);
	}
	catkeyPid = fork();
	switch (catkeyPid) {
	case -1:
		perror("Error forking cat key-file");
		AtExit(2);
		break;
	case 0:
		close(catkey_descs[0]);
		if (dup2(catkey_descs[1], 1) == -1) {
			perror("Error while redirecting cat key input");
			AtExit(3);
		}
		close(catkey_descs[1]);
		memset(buf, '\0', BUFSIZE);
		strcat(buf, path);
		strcat(buf, ".key");
		execlp("cat", "cat", buf, NULL);
		break;
	}
	
	if (pipe(catfile_descs) == -1) {
		perror("Error creating file pipe");
		AtExit(4);
	}
	catfilePid = fork();
	switch (catfilePid) {
	case -1:
		perror("Error forking the filewalker");
		AtExit(5);
		break;
	case 0:
		close(catfile_descs[0]);
		if (dup2(catfile_descs[1], 1) == -1) {
			perror("Error while redirecting cat file input");
			AtExit(6);
		}
		close(catfile_descs[1]);
		execlp("cat", "cat", path, NULL);
		break;
	default:
		close(catfile_descs[1]);
		sleep(1);
		
		char xor[BUFSIZE];
		int filereaded;
		int keyreaded;
		int i;
		int xorfile;
	
		memset(buf, '\0', BUFSIZE);
		memset(xor, '\0', BUFSIZE);
		strcat(buf, path);
		buf[strlen(buf)-8] = '\0';
		strcat(buf, ".decrypted");
		xorfile = open(buf, O_RDWR|O_CREAT, 0600);
		if (xorfile == -1) {
			perror("Failed to create decrypted file");
			AtExit(7);
		}
		memset(buf, '\0', BUFSIZE);
		while ((filereaded = read(catfile_descs[0], buf, BUFSIZE)) > 0) {
			keyreaded = read(catkey_descs[0], xor, filereaded);
		//	printf("filereaded = %i, keyreaded = %i\n", filereaded, keyreaded);
			if (filereaded != keyreaded) {
				printf("Seems like shit.");
				printf(" Results will be incorrect. Stopping.\n");
				AtExit(8);
			}
			for (i = 0; i < filereaded; i++)
				buf[i] = buf[i] ^ xor[i];
			i = write(xorfile, buf, filereaded);
			if (i != filereaded) {
				perror("Error while writing into decrypted file");
				AtExit(9);
			}
			memset(buf, '\0', BUFSIZE);
			memset(xor, '\0', BUFSIZE);
		}
		
		close(catfile_descs[0]);
		close(catkey_descs[0]);
		close(xorfile);
		
		AtExit(0);
		
		break;
	}
}
