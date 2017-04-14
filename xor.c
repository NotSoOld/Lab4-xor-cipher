#include "lab4.h"

pid_t genPid;

void AtExit(int exitCode)
{
	kill(genPid, SIGINT);
	// Parent waits for child's command completion.
	wait(NULL);
	exit(exitCode);
}

int main(int argc, char *argv[])
{
	int gen_descs[2];
	int cat_descs[2];
	pid_t catPid;
	char *out;
	char path[BUFSIZE];
	char buf[BUFSIZE];
	
	if(argc != 3) {
		printf("Usage: ./xor \"generator-program [path-to-key-file]\"");
		printf(" \"program-to-read-file [path-to-file]\"\n");
		printf("Not enough arguments. Stopping.\n");
		AtExit(1);
	}
	// Create a pipe to connect generator and cipher.
	if (pipe(gen_descs) == -1) {
		perror("Error creating generator pipe");
		AtExit(2);
	}
	// Take generator params.
	char *genargs[3];
	genargs[0] = strtok(argv[1], " ");
	genargs[1] = strtok(NULL, " ");
	genargs[2] = NULL;
	// Take catalog params.
	char *catargs[3];
	catargs[0] = strtok(argv[2], " ");
	catargs[1] = strtok(NULL, " ");	
	catargs[2] = NULL;
	// Fork and run the generator.
	genPid = fork();
	switch (genPid) {
	case -1:
		perror("Error forking the generator");
		AtExit(3);
		break;
	case 0:
		// Before we run generator, we need to redirect its output to our pipe.
		close(gen_descs[0]);
		// Duplicate our pipe input descriptor as stdout.
		if (dup2(gen_descs[1], 1) == -1) {
			perror("Error while redirecting generator input");
			AtExit(4);
		}
		close(gen_descs[1]);
		if (execvp(genargs[0], genargs) == -1) {
			printf("Error while executing key generator!\n");
			perror("Purpose");
			AtExit(5);
		}
		break;
	}
	// Create a pipe for cat process.
	if (pipe(cat_descs) == -1) {
		perror("Error creating catalog pipe");
		AtExit(6);
	}
	// Fork and run cat-filewalker.
	catPid = fork();
	switch (catPid) {
	case -1:
		perror("Error forking the filewalker");
		AtExit(7);
		break;
	case 0:
		// Before we run cat, we need to redirect its output to our pipe.
		close(cat_descs[0]);
		// Duplicate our pipe input descriptor as stdout.
		if (dup2(cat_descs[1], 1) == -1) {
			perror("Error while redirecting cat input");
			AtExit(8);
		}
		close(cat_descs[1]);
		if (execvp(catargs[0], catargs) == -1) {
			printf("Error while executing file reader!\n");
			perror("Purpose");
			AtExit(9);
		}
		break;
	default:
		close(cat_descs[1]);
		sleep(1);		// Wait a bit for generator.
		
		char xor[BUFSIZE];
		int readed;
		int genreaded;
		int i;
		int xorfile;
		int xorkey;
	
		// Get ready to write ciphed file and key to decipher it later.
		memset(buf, '\0', BUFSIZE);
		memset(xor, '\0', BUFSIZE);
		if (catargs[1] != NULL)
			strcat(buf, catargs[1]);
		else
			strcat(buf, "out");
		strcat(buf, ".xor");
		xorfile = open(buf, O_RDWR|O_CREAT, 0600);
		if (xorfile == -1) {
			perror("Failed to create xor-file");
			AtExit(10);
		}
		strcat(buf, ".key");
		xorkey = open(buf, O_RDWR|O_CREAT, 0600);
		if (xorkey == -1) {
			perror("Failed to create key-file");
			AtExit(11);
		}
		memset(buf, '\0', BUFSIZE);
		// While we haven't read the entire file...
		while (1) {
			genreaded = read(gen_descs[0], xor, BUFSIZE);
			readed = read(cat_descs[0], buf, genreaded);
			// Apply XOR operation and save readed key.
			for (i = 0; i < readed; i++)
				buf[i] = buf[i] ^ xor[i];
			i = write(xorfile, buf, readed);
			if (i != readed) {
				perror("Error while writing into xor-file");
				AtExit(12);
			}
			i = write(xorkey, xor, readed);
			if (i != readed) {
				perror("Error while writing into key-file");
				AtExit(13);
			}
			// Exit when we read the entire file.
			if (genreaded != readed)
				break;
			memset(buf, '\0', BUFSIZE);
			memset(xor, '\0', BUFSIZE);
		}
		
		close(cat_descs[0]);
		close(xorfile);
		close(xorkey);
		
		AtExit(0);
	}
}
