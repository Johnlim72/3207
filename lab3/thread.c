#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

pthread_t threads[8]; //Makes an array of 8 thread IDs.
int total_signal_count = 0;

void* generator(); //Sends an amount of signals to handler and reporter threads.
void delay(); //Delay for .01 to .1 seconds.
void* handler1(); //Handles SIGUSR1 signals sent by generator threads.
void* handler2(); //Handles SIGUSR2 signals sent by generator threads.
void* reporter(); //Handles SIGUSR1 and SIGUSR2; reports average time in between each signal type recieved.

int reporter_sigs = 0; //Total signals recieved by reporter.
int sig1_sent = 0; //Total SIGUSR1 signals sent by generator.
int sig2_sent = 0; //Total SIGUSR2 signals sent by generator.
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
		printf("created handler1 threads #%d\n", i);
	}

	//Creates 2 threads for handler2.
	for(i = 2; i < 4; i++) { 
		if (pthread_create(&threads[i], NULL, handler2, NULL) != 0) {
			printf("error pthread_create handler2\n");
		}
		printf("created handler2 threads #%d\n", i);
	}

	//Creates a thread for reporter.
	if (pthread_create(&threads[7], NULL, reporter, NULL) != 0) {
		printf("error pthread_create reporter\n");
	}
	printf("created reporter threads\n");
	
	sleep(1);

	srand(time(NULL));
	
	//Creates 3 threads for generator.
	for(i = 4; i < 7; i++) {
		if (pthread_create(&threads[i], NULL, generator, NULL) != 0) {
			printf("error pthread_create generator\n");	
		}
		printf("created generator thread #%d\n", i);
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
	while(total_signal_count < 1000) { 
		int r = (rand() % 2) +1;
		if (r == 1) {
			int i;
			for (i =0; i<2; i++) {
				pthread_kill(threads[i], SIGUSR1); 
				pthread_mutex_lock(&sem1);
				sig1_sent++;
				pthread_mutex_unlock(&sem1);
			}	
			pthread_kill(threads[7], SIGUSR1);
		}
		if (r == 2) {
			int i;
			for (i =2; i<4; i++) {
				pthread_kill(threads[i], SIGUSR2);
				pthread_mutex_lock(&sem2);
				sig2_sent++;
				pthread_mutex_unlock(&sem2);
				}
			pthread_kill(threads[7], SIGUSR2);
		}	

	pthread_mutex_lock(&sem5);
	total_signal_count++;
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
			sig1_rec++;
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
			sig2_rec++;
			pthread_mutex_unlock(&sem4);
		} else if (s == -1) {
			break;
		}
	}
}

void* reporter() {
	struct timespec timeout;
	timeout.tv_sec = 3;
	timeout.tv_nsec = 0;
	
	struct timeval tv1;
	unsigned int times1[10]; //times for sigusr1	
	unsigned int times2[10]; //times for sigusr2

	sigemptyset(&set3);
	sigaddset(&set3, SIGUSR1);
	sigaddset(&set3, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &set3, NULL);
	
	int i = 0; //times1 counter
	int t = 0; //times2 counter
	int s;
	int which;

	int sig1fortime = 0;
	int sig2fortime = 0;

	int z = 0;

	while (1) {
		if ((s = sigtimedwait(&set3, NULL, &timeout)) > 0) {
			gettimeofday(&tv1, NULL);
			printf("time %d = %d microsec\n", z, ((unsigned int)tv1.tv_sec * 1000000) + (unsigned int)tv1.tv_usec);
			z++;

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
			reporter_sigs++;
			pthread_mutex_unlock(&sem6);
		
			printf("sig1_sent = %d, sig2_sent = %d\n", sig1_sent, sig2_sent);
			printf("sig1_rec = %d, sig2_rec = %d\n", sig1_rec, sig2_rec);	
			printf("reporter_sigs = %d\n", reporter_sigs);
			
			if (reporter_sigs % 10 == 0) {
				unsigned int avg1 = 0; //avg for sigusr1
				unsigned int avg2 = 0; //avg for sigusr2
				unsigned int diff1; //time difference for sigusr1
				unsigned int diff2; //time difference for sigusr2
				
				int j;
				for (j =0; j < sig1fortime-1; j++) {
					diff1 = times1[j+1] - times1[j];
					avg1+= diff1;
				}
				int k;
				for (k =0; k < sig2fortime-1; k++) {
					diff2 = times2[k+1] - times2[k];
					avg2+= diff2;
				}
				
				avg1 = avg1 / sig1fortime;
				printf("sig1fortime = %d, sig2fortime = %d\n", sig1fortime, sig2fortime);
				printf("avg1 = %d microseconds\n", avg1);
				avg2 = avg2 / sig2fortime;
				printf("avg2 = %d microseconds\n", avg2);

				i = 0;	
				t = 0;	  
				sig1fortime = 0;
				sig2fortime = 0;
				avg1 = 0;
				avg2 = 0;
				
			}

		} else if (s == -1) {
			break;
		}
	}
}


