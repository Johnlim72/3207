#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*Writes times recorded  before fork() function into a file then executes "Application" program for each of the 1st and 2nd child processes, all 100 times*/
int main() {
	int i;
	for (i =1; i <=100; i++) {	
		struct timeval tv1;
		struct timeval tv2;
		gettimeofday(&tv1, NULL);

        	printf("Before first fork (seconds): %d\n", (int)tv1.tv_sec);
        	printf("Before first fork (microseconds): %d\n", (int)tv1.tv_usec);

        	FILE* fp3 = fopen("times2.csv", "a");
       		fprintf(fp3, "%d, %d\n", (int)tv1.tv_sec, (int)tv1.tv_usec);
        	fclose(fp3);

		printf("I am about to fork\n");
		pid_t pid = fork();

		if (pid == 0) { //child process
			printf("I am in 1st child process\n");
			execlp("./Application.out", "./Application.out", NULL);
			printf("This will not be printed if the exec call succeeds\n");
		} else { //parent
			printf("I am in parent process\n");
	        	gettimeofday(&tv2, NULL);
                	printf("Before second fork (seconds): %d\n", (int)tv2.tv_sec);
                	printf("Before second fork (microseconds): %d\n", (int)tv2.tv_usec);

                	FILE* fp3 = fopen("times2.csv", "a");
                	fprintf(fp3, "%d, %d\n",(int)tv2.tv_sec, (int)tv2.tv_usec);
                	fclose(fp3);

			pid_t pid2 = fork();
			if (pid2 == 0) { // 2nd child process
				printf("I am in second child process\n");
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
