#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

pthread_t threads[8];
void* generator();
void* handler1();
void* handler2();
void* reporter();
int reporter_sigs = 0;
int sig1_sent = 0;
int sig2_sent = 0;
int sig1_rec = 0;
int sig2_rec = 0;
sem_t sem1, sem2, sem3, sem4, sem5, sem6;
int total_signal_count = 0;
sigset_t set1, set2, set3;

int main() {
	sem_init(&sem1, 0, 1);
	sem_init(&sem2, 0, 1);
	sem_init(&sem3, 0, 1);
	sem_init(&sem4, 0, 1);
	sem_init(&sem5, 0, 1);
	sem_init(&sem6, 0, 1);
	
	int i;

	for(i = 0; i < 2; i++) {
		if (pthread_create(&threads[i], NULL, handler1, NULL) != 0) {
			printf("error pthread_create handler1\n");
		}
		printf("created handler1 threads #%d\n", i);
	}

	for(i = 2; i < 4; i++) {
		if (pthread_create(&threads[i], NULL, handler2, NULL) != 0) {
			printf("error pthread_create handler2\n");
		}
		printf("created handler2 threads #%d\n", i);
	}

	if (pthread_create(&threads[7], NULL, reporter, NULL) != 0) {
		printf("error pthread_create reporter\n");
	}
	printf("created reporter threads\n");
	
	for(i = 4; i < 7; i++) {
		if (pthread_create(&threads[i], NULL, generator, NULL) != 0) {
			printf("error pthread_create generator\n");	
		}
		printf("created generator thread #%d\n", i);
	}

	for(i=0; i < 8; i++) { //change after reporter threads made
		pthread_join(threads[i], NULL);
	} 

	printf("\ntotal sigs sent to handlers = %d\n", sig1_sent + sig2_sent);
	printf("total sigs rec by handlers = %d\n", sig1_rec + sig2_rec);
	return 0;
}

void* generator() {
	printf("total_signal_count = %d\n", total_signal_count);
	srand(time(NULL));
	while(total_signal_count < 10) {
		int r = (rand() % 2) +1;
		if (r == 1) {
			int i;
			for (i =0; i<2; i++) {
				if (pthread_kill(threads[i], SIGUSR1) != 0) {
					printf("pthread_kill sig1 to handler error\n");
				} else {
					printf("pthread_kill sig1 to handler success\n");
					sem_wait(&sem1);
					sig1_sent++;
					sem_post(&sem1);
				}
			}
			if (pthread_kill(threads[7], SIGUSR1) != 0) {
				printf("pthread_kill sig1 to rep error\n");
			} else {
				printf("pthread_kill sig1 to rep success\n");
			}
			//	printf("sig1sent = %d\n", sig1_sent);
		}
		if (r == 2) {
			int i;
			for (i =2; i<4; i++) {
				if (pthread_kill(threads[i], SIGUSR2) != 0) {
					printf("pthread_kill sig2 to handler error\n");
				} else {
					printf("pthread_kill sig2 to handler success\n");
				sem_wait(&sem2);
				sig2_sent++;
				sem_post(&sem2);
				}
			}	
			if (pthread_kill(threads[7], SIGUSR2) != 0) {
				printf("pthread_kill sig2 to rep error\n");
			} else {
				printf("pthread_kill sig2 to rep success\n");
			}
			//	printf("sig2sent = %d\n", sig2_sent);
		}	
	sem_wait(&sem5);
	total_signal_count++;
	sem_post(&sem5);
	double rand_time = (double)(((rand() % 91) + 10) /1000);
	sleep(1);
	}
}

void* handler1() {
	struct timespec timeout;
	timeout.tv_sec = 5;
	timeout.tv_nsec = 0;

//	signal(SIGUSR2, SIG_IGN);
//	signal(SIGUSR1, handler1);

	sigemptyset(&set1);
	if (sigaddset(&set1, SIGUSR1) != 0) {
		printf("error sigaddset handler1\n");
	}
	pthread_sigmask(SIG_BLOCK, &set1, NULL);
 	int s;
	while (1) {
		if ((s = sigtimedwait(&set1, NULL, &timeout)) == 10) {
			sem_wait(&sem3);
			sig1_rec++;
			sem_post(&sem3);
		//	printf("sig1_rec = %d\n", sig1_rec);
		} else if (s == -1) {
			break;
		}
	}
	
	pthread_sigmask(SIG_UNBLOCK, &set1, NULL);	
	pthread_exit(NULL);
}

void* handler2() {
	struct timespec timeout;
	timeout.tv_sec = 5;
	timeout.tv_nsec = 0;
	
//	signal(SIGUSR1, SIG_IGN);
//	signal(SIGUSR2, handler2);

	sigemptyset(&set2);
	if (sigaddset(&set2, SIGUSR2) != 0) {
		printf("error sigaddset handler2\n");
 	}
	pthread_sigmask(SIG_BLOCK, &set2, NULL);
	int s;
	while (1) {
		if ((s = sigtimedwait(&set2, NULL, &timeout)) == 12) {
			sem_wait(&sem4);
			sig2_rec++;
			sem_post(&sem4);
		//	printf("sig2_rec = %d\n", sig2_rec);
		} else if (s == -1) {
			break;
		}
	}
	pthread_sigmask(SIG_UNBLOCK, &set2, NULL);
	pthread_exit(NULL);
}

void* reporter() {
	struct timespec timeout;
	timeout.tv_sec = 5;
	timeout.tv_nsec = 0;
	
	struct timeval tv1;
	int times[10];
	
	sigemptyset(&set3);
	sigaddset(&set3, SIGUSR1);
	sigaddset(&set3, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &set3, NULL);
	
	int s;
	int i = 0;
	
	while (1) {
		if ((s = sigtimedwait(&set3, NULL, &timeout)) > 0) {
			gettimeofday(&tv1, NULL);
		//	printf("time %d = %d\n", i, (int)tv1.tv_sec);
			times[i] = (int)tv1.tv_sec;
			i++;

			sem_wait(&sem6);
			reporter_sigs++;
			sem_post(&sem6);
		
			printf("sig1_sent = %d, sig2_sent = %d\n", sig1_sent, sig2_sent);
			printf("sig1_rec = %d, sig2_rec = %d\n", sig1_rec, sig2_rec);	
			printf("reporter_sigs = %d\n", reporter_sigs);
			
			if (reporter_sigs % 10 == 0) {
				i = 0;
			}

		} else if (s == -1) {
			break;
		}
	}

	pthread_sigmask(SIG_UNBLOCK, &set3, NULL);
	pthread_exit(NULL);
}


