#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 

int main() {
	printf("Hello World\n");
	printf("I am about to fork()!\n");
	pid_t pid = fork();

	if (pid == 0) {
		printf("I am in child process\n");
		exit(0);
	} else {
		printf("I am in parent process\n");
	}
	printf("End of current process\n");

}
