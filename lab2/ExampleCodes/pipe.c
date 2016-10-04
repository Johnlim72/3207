#include <stdio.h>
#include <unistd.h>
int main() {
	int pipefd[2];
	pipe(pipefd);
	int pid = fork();
	if  (pid == 0) {
		dup2(pipefd[0], 0); //input wil be replaced by pipefd[0].
		close(pipefd[1]);
		char str[100];
		gets(str);
		execvp("wc", "wc");
		printf("%s\n", str);
	} else {
		dup2(pipefd[1], 1); //output will be replace by pipefd[1].
		close(pipefd[0]);
		execvp("ls", "ls");
		wait(NULL);
	}
	printf("hi");
	return 0;
}
