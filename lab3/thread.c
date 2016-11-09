#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

pthread_t threads[8]; //Makes an array of 8 thread IDs.
int total_signal_count = 0; //Total signals sent by generator.

void* generator(); //Sends an amount of signals to handler and reporter threads.
void delay(); //Delay for .01 to .1 seconds.
void* handler1(); //Handles SIGUSR1 signals sent by generator threads.
void* handler2(); //Handles SIGUSR2 signals sent by generator threads.
void* reporter(); //Handles SIGUSR1 and SIGUSR2 signals send by generator threads; reports average time in between each signal type recieved.

int reporter_sigs = 0; //Total signals recieved by reporter from generator.
int sig1_sent = 0; //Total SIGUSR1 signals sent by generator to handler1 threads.
int sig2_sent = 0; //Total SIGUSR2 signals sent by generator to handler2 threads.
int sig1_rec = 0; //Total SIGUSR1 signals recieved by handler1.
int sig2_rec = 0; //Total SIGUSR2 signals recieved by handler2.

pthread_mutex_t sem1, sem2, sem3, sem4, sem5, sem6; //Declares semaphores.
sigset_t set1, set2, set3; //Declares sets for signals.

int main() {
	int i;
	
	//Creates 2 threads for handler1.
	for(i = 0; i < 2; i++) { 
		if (pthread_create(&threads[i], NULL, handler1, NULL) != 0) {
			printf("error pthread_create handler1\n");
		}
		printf("Created handler1 threads #%d\n", i);
	}

	//Creates 2 threads for handler2.
	for(i = 2; i < 4; i++) { 
		if (pthread_create(&threads[i], NULL, handler2, NULL) != 0) {
			printf("error pthread_create handler2\n");
		}
		printf("Created handler2 threads #%d\n", i);
	}

	//Creates a thread for reporter.
	if (pthread_create(&threads[7], NULL, reporter, NULL) != 0) {
		printf("error pthread_create reporter\n");
	}
	printf("Created reporter threads\n");
	
	sleep(1);

	srand(time(NULL));
	
	//Creates 3 threads for generator.
	for(i = 4; i < 7; i++) {
		if (pthread_create(&threads[i], NULL, generator, NULL) != 0) {
			printf("error pthread_create generator\n");	
		}
		printf("Created generator thread #%d\n", i);
	}

	//Waits for each thread to finish.
	for(i=0; i < 8; i++) { 
		pthread_join(threads[i], NULL);
	} 

	printf("\nTotal sigs sent to handlers = %d\n", sig1_sent + sig2_sent);
	printf("Total sigs recieved by handlers = %d\n", sig1_rec + sig2_rec);

	return 0;
}

//Sends an amount of signals to handler and reporter threads.
void* generator() {
	while(total_signal_count < 20000) { 
		int r = (rand() % 2) +1; //Chooses random number, either 1 or 2.
		if (r == 1) { //If random number equals 1, SIGUSR1 will be sent to handler1 threads.
			int i;
			for (i =0; i<2; i++) { //Sends SIGUSR1 to handler1 threads.
				pthread_kill(threads[i], SIGUSR1); 
				pthread_mutex_lock(&sem1);
				sig1_sent++; //Increment total SIGUSR1 signals sent.
				pthread_mutex_unlock(&sem1);
			}	
			pthread_kill(threads[7], SIGUSR1); //Sends SIGUSR1 to reporter thread.
		}
		if (r == 2) { //If random number equals 2, SIGUSR2 will be sent to handler2 threads.
			int i;
			for (i =2; i<4; i++) { //Sends SIGUSR2 to handler2 threads.
				pthread_kill(threads[i], SIGUSR2);
				pthread_mutex_lock(&sem2);
				sig2_sent++; //Increment total SIGUSR2 signals sent.
				pthread_mutex_unlock(&sem2);
			}
			pthread_kill(threads[7], SIGUSR2); //Sends SIGUSR2 to reporter thread.
		}	

	pthread_mutex_lock(&sem5);
	total_signal_count+= 2; //Increments total signals sent by generator thread (total_signal_count).
	pthread_mutex_unlock(&sem5);
	
	delay();

	}
}

//Delays for .01 to .1 seconds.
void delay() {
	struct timespec ts;
	double seconds = (((double) rand()) / ((double)RAND_MAX) * 90000000) + 10000000;
	ts.tv_sec = 0;
	ts.tv_nsec = seconds;
	nanosleep(&ts, NULL);
}

//Handles SIGUSR1 signals sent by generator threads.
void* handler1() {
	struct timespec timeout;
	timeout.tv_sec = 3;
	timeout.tv_nsec = 0;

	sigemptyset(&set1);
	sigaddset(&set1, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &set1, NULL);
 
	int s;
	while (1) {
		if ((s = sigtimedwait(&set1, NULL, &timeout)) > 0) {
			pthread_mutex_lock(&sem3);
			sig1_rec++; //Increment total SIGUSR1 signals recieved by handler1 threads.
			pthread_mutex_unlock(&sem3);
		} else if (s == -1) {
			break;
		}
	}
}

//Handles SIGUSR2 signals sent by generator threads.
void* handler2() {
	struct timespec timeout;
	timeout.tv_sec = 3;
	timeout.tv_nsec = 0;
	
	sigemptyset(&set2);
	sigaddset(&set2, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &set2, NULL);

	int s;
	while (1) {
		if ((s = sigtimedwait(&set2, NULL, &timeout)) > 0) {
			pthread_mutex_lock(&sem4);
			sig2_rec++; //Increment total SIGUSR2 signals received by handler2 threads.
			pthread_mutex_unlock(&sem4);
		} else if (s == -1) {
			break;
		}
	}
}

//Handles SIGUSR1 and SIGUSR2 signals sent by generator threads; reports average time in between each signal type recieved.
void* reporter() {
	struct timespec timeout;
	timeout.tv_sec = 3;
	timeout.tv_nsec = 0;
	
	//Reports to .csv file
	FILE *fp = fopen("test_results.csv", "w"); 
	fprintf(fp, "Reporter Signals, TIME(Sec), TIME(Microsec), SIGUSR1 Count, AVG TIME 1, SIGUSR2 Count, AVG TIME 2\n");
	
	struct timeval tv1;
	unsigned int times1[10]; //Time stamps for SIGUSR1.	
	unsigned int times2[10]; //Time stamps for SIGUSR2.
	int i = 0; //times1 array counter
	int t = 0; //times2 array counter
	int sig1fortime = 0;
	int sig2fortime = 0;

	sigemptyset(&set3);
	sigaddset(&set3, SIGUSR1);
	sigaddset(&set3, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &set3, NULL);
	
	int s;
	while (1) {
		if ((s = sigtimedwait(&set3, NULL, &timeout)) > 0) {
			gettimeofday(&tv1, NULL);

			if (s == 10) {
				times1[i] = ((unsigned int)tv1.tv_sec * 1000000) + (unsigned int) tv1.tv_usec;
				i++;
				sig1fortime++;
			} else if (s == 12) {
				times2[t] = ((unsigned int)tv1.tv_sec * 1000000) + (unsigned int) tv1.tv_usec;
				t++;
				sig2fortime++;
			}			
	
			pthread_mutex_lock(&sem6);
			reporter_sigs++; //increments total signals received by reporter thread.
			pthread_mutex_unlock(&sem6);
					
			if (reporter_sigs % 10 == 0) {
				unsigned int avg1 = 0; //average for sigusr1
				unsigned int avg2 = 0; //average for sigusr2
				unsigned int diff1; //time difference for sigusr1
				unsigned int diff2; //time difference for sigusr2
				
				//Calculates average time in between signals sent for each signal type
				int j;
				if (sig1fortime > 1) {
					for (j =0; j < sig1fortime-1; j++) {
						diff1 = times1[j+1] - times1[j];
						avg1+= diff1;
					}
					avg1 = avg1 / sig1fortime;
				} else if (sig1fortime <= 1) {
					diff1 = 0;
					avg1 = 0;
				} 
				int k;
				if (sig2fortime > 1) {
					for (k =0; k < sig2fortime-1; k++) {
						diff2 = times2[k+1] - times2[k];
						avg2+= diff2;
					}
					avg2 = avg2 / sig2fortime;
				} else if (sig2fortime <= 1 ) {
					diff2 = 0;
					avg2 = 0;
				} 

				//Reports result for the iteration to .csv file
				fprintf(fp, "%d, %d, %d, %d, %d, %d, %d\n", reporter_sigs, (int)tv1.tv_sec, (int)tv1.tv_usec, sig1fortime, avg1, sig2fortime, avg2);				
				i = 0;	
				t = 0;	  
				sig1fortime = 0;
				sig2fortime = 0;
				avg1 = 0;
				avg2 = 0;
			
			}
		} else if (s == -1) {
			printf("reporter sigs = %d\n", reporter_sigs);
			fclose(fp);	
			break;
		}
	}
}
