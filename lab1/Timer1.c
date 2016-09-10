#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	pid_t pid = fork();

	if (pid == 0) { //child process
		printf("I am in child process\n");
       	        printf("Before exec (seconds): %d\n", (int)tv.tv_sec);
	        printf("Before exec (microseconds): %d \n", (int)tv.tv_usec);

		execlp("./Application.out", "./Application.out", NULL);
		printf("This will not be printed if the exec call succeeds\n");
	} else { //parent
		printf("I am in parent process\n");
		waitpid(pid, NULL, 0);
	}
	printf("End of current process\n");
}
