#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
	int i;
	for (i =1; i <= 100; i++) {
	
	
	struct timeval tv1;
	struct timeval tv2;
	printf("I am about to fork\n");
	pid_t pid = fork();

	if (pid == 0) { //child process
		printf("I am in 1st child process\n");
		gettimeofday(&tv1, NULL);
	
       	        printf("Before first exec (seconds): %d\n", (int)tv1.tv_sec);
	        printf("Before first exec (microseconds): %d \n", (int)tv1.tv_usec);
		
		//FILE* fp3 = fopen("times2.csv", "a");
		//fprintf(fp3, "%d, %d\n", tv1.tv_sec, tv1.tv_usec);
		//fclose(fp3);

		execlp("./Application.out", "./Application.out", NULL);
		printf("This will not be printed if the exec call succeeds\n");
	} else { //parent
		printf("I am in parent process\n");
		pid_t pid2 = fork();
		
		if (pid2 == 0) { // 2nd child process
			printf("I am in second child process\n");
			gettimeofday(&tv2, NULL);
			
			printf("Before second exec (seconds): %d\n", (int)tv2.tv_sec);
			printf("Before second exec (microseconds): %d\n", (int)tv2.tv_usec);
			
			FILE* fp3 = fopen("times2.csv", "a");
			fprintf(fp3, "%d, %d\n", tv2.tv_sec, tv2.tv_usec);
			fclose(fp3);
	
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
}
