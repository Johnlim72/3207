#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main(){
	char hostname[1024];
	if (gethostname(hostname, sizeof(hostname)) != -1) {
		printf("%s>\n", hostname);
	} else {
		perror("gethostname error");
	}


	char str[] = "command -arg -args < .txt";
	
	char **args = malloc(100 *sizeof(char*));
	int i =0;
	int j =0;
	char *arg;

	arg = strtok(str, " ");
	while (arg != NULL) {
		args[j] = arg;
		printf("%s\n", args[j]);
		arg = strtok(NULL, " ");
		j++;
	} 

	

	int fd[2];
	int bytes_read;
	pid_t pid;
	char buf[1024];
	char *who="I'm the child: ";

	if (pipe(fd)==-1) {
		//fprintf(stderr, "pipe error: %s\n", strerror(errno));
		return 0;
	}
	if ((pid=fork())==-1) {
		//fprintf(stderr, "fork error: %s\n", strerror(errno));
		return 0;
	} else if (pid==0) {
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		if ((bytes_read=read(STDIN_FILENO, buf, 1024))>0) {
			write(STDOUT_FILENO, who, strlen(who));
		write(STDOUT_FILENO, buf, 1024);
		}
	} else {
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		printf("Who lives in a pineapple under the sea?\n");
	}	
	return 0;
}


