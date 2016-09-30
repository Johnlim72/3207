#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*Writes times recorded before fork() function into a file and executes "Application" program in child process, all 100 times*/
int main() {
	int i;
	for (i = 1; i <= 100; i++) {
	
		struct timeval tv;
		gettimeofday(&tv, NULL);
		printf("Before fork (seconds): %d\n", (int)tv.tv_sec);
        	printf("Before fork (microseconds): %d \n", (int)tv.tv_usec);

        	FILE* fp2 = fopen("times1.csv", "a");
        	fprintf(fp2, "%d, %d\n",(int)tv.tv_sec, (int)tv.tv_usec);
        	fclose(fp2);

		pid_t pid = fork();
		if (pid == 0) { //child process
			printf("I am in child process\n");
			execlp("./Application.out", "./Application.out", NULL);
			printf("This will not be printed if the exec call succeeds\n");
		} else { //parent process
			printf("I am in parent process\n");
			waitpid(pid, NULL, 0);
		}
		printf("End of current process\n");
	}	
}
