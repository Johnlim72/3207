#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("I am about to fork\n");
	pid_t pid = fork();

	if (pid == 0) { //child process
		printf("I am in 1st child process\n");
       	        printf("Before first exec (seconds): %d\n", (int)tv.tv_sec);
	        printf("Before first exec (microseconds): %d \n", (int)tv.tv_usec);
		
		FILE* fp3 = fopen("times2.csv", "a");
		fprintf(fp3, "%d, %d\n", tv.tv_sec, tv.tv_usec);
		fclose(fp3);

		execlp("./Application.out", "./Application.out", NULL);
		printf("This will not be printed if the exec call succeeds\n");
	} else { //parent
		printf("I am in parent process\n");
		pid_t pid2 = fork();
	        gettimeofday(&tv, NULL);
		
		if (pid2 == 0) { // 2nd child process
			printf("I am in second child process\n");
			
			FILE* fp3 = fopen("times2.csv", "a");
                	fprintf(fp3, "%d, %d\n", tv.tv_sec, tv.tv_usec);
        	        fclose(fp3);
	
			printf("Before second exec (seconds): %d\n", (int)tv.tv_sec);
			printf("Before second exec (microseconds): %d\n", (int)tv.tv_usec);
			
			execlp("./Application.out", "./Application.out", NULL);
			printf("This will not be printed if the exec call succeeds");
		} else { //parent process
			printf("I am in parent process\n");
			waitpid(pid2, NULL, 0);
		}
		waitpid(pid, NULL, 0);
	}
	printf("End of current process\n");
}
