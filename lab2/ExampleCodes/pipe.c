#include <stdio.h>
#include <unistd.h>
int main() {
	int pipefd[2];
	pipe(pipefd);
	int pid = fork();
	if  (pid == 0) {
		dup2(pipefd[0], 0); //input wil be replaced by pipefd[0].
		char str[100];
		gets(str);
		printf("This is in child process, reading from the pipe: \n");
		printf("%s\n", str);
	} else {
		dup2(pipefd[1], 1); //output will be replace by pipefd[1].
		printf("This is something from the parent process\n"); //write to pipe
	}
	return 0;
}
