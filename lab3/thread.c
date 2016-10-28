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
int sig1_sent = 0;
int sig2_sent = 0;
int sig1_rec = 0;
int sig2_rec = 0;
sem_t sem1, sem2, sem3, sem4, sem5;
int total_signal_count = 0;
sigset_t set1, set2;

int main() {
	sem_init(&sem1, 0, 1);
	sem_init(&sem2, 0, 1);
	sem_init(&sem3, 0, 1);
	sem_init(&sem4, 0, 1);
	sem_init(&sem5, 0, 1);

	int i;

	for(i = 3; i < 5; i++) {
		if (pthread_create(&threads[i], NULL, handler1, NULL) != 0) {
			printf("error pthread_create handler1\n");
		}
		printf("created handler1 threads #%d\n", i);
	}
	sleep(1);
	for(i = 5; i < 7; i++) {
		if (pthread_create(&threads[i], NULL, handler2, NULL) != 0) {
			printf("error pthread_create handler2\n");
		}
		printf("created handler2 threads #%d\n", i);
	}
	sleep(1);
	for(i = 0; i < 3; i++) {
		if (pthread_create(&threads[i], NULL, generator, NULL) != 0) {
			printf("error pthread_create generator\n");	
		}
		printf("created generator thread #%d\n", i);
	}
	sleep(1);
	if (pthread_create(&threads[7], NULL, reporter, NULL) != 0) {
		printf("error pthread_create reporter\n");
	}

	for(i=0; i < 8; i++) {
		pthread_join(threads[i], NULL);
	}
	
	printf("created reporter threads\n");
	return 0;
}

void* generator() {
	printf("in generator function\n");
	printf("total_signal_count = %d\n", total_signal_count);
	srand(time(NULL));
	while(total_signal_count < 2) {
		int r = (rand() % 2) +1;
		if (r == 1) {
			int i;
			for (i =3; i<5; i++) {
				if (pthread_kill(threads[i], SIGUSR1) != 0) {
					printf("pthread_kill sig1 to handler error\n");
				} else {
					printf("pthread_kill sig1 to handler success\n");
				}
				/*if (pthread_kill(threads[7], SIGUSR1) != 0) {
					printf("pthread_kill sig1 to rep error\n");
				} else {
					printf("pthread_kill sig1 to rep success\n");
				}*/
				sem_wait(&sem1);
				sig1_sent++;
				sem_post(&sem1);
				printf("sig1sent = %d\n", sig1_sent);
			}
		} else if (r == 2) {
			int i;
			for (i =5; i<7; i++) {
				if (pthread_kill(threads[i], SIGUSR2) != 0) {
					printf("pthread_kill sig2 to handler error\n");
				} else {
					printf("pthread_kill sig2 to handler success\n");
				}
				/*if (pthread_kill(threads[7], SIGUSR2) != 0) {
					printf("pthread_kill sig2 to rep error\n");
				} else {
					printf("]thread_kill sig2 to rep success\n");
				}	*/
				sem_wait(&sem2);
				sig2_sent++;
				sem_post(&sem2);
				printf("sig2sent = %d\n", sig2_sent);
			}
		}
	sem_wait(&sem5);
	total_signal_count++;
	sem_post(&sem5);
	double rand_time = (double)(((rand() % 91) + 10) /1000);
	sleep(1);
	}
	pthread_exit(NULL);
}

void* handler1() {
	struct timespec timeout;
	timeout.tv_sec = 2;
	timeout.tv_nsec = 0;

	sigemptyset(&set1);
	if (sigaddset(&set1, SIGUSR1) != 0) {
		printf("error sigaddset handler1\n");
	}
	pthread_sigmask(SIG_BLOCK, &set1, NULL);
 	int s;
	while (1) {
		if ((s = sigtimedwait(&set1, NULL, &timeout)) > 0) {
			sem_wait(&sem3);
			sig1_rec++;
			sem_post(&sem3);
			printf("sig1_rec = %d\n", sig1_rec);
		} else if (s == -1) {
			break;
		}
	}
	
	//pthread_sigmask(SIG_UNBLOCK, &set1, NULL);	
	pthread_exit(NULL);
}

void* handler2() {
	struct timespec timeout;
	timeout.tv_sec = 2;
	timeout.tv_nsec = 0;
	
	sigemptyset(&set2);
	if (sigaddset(&set2, SIGUSR2) != 0) {
		printf("error sigaddset handler2\n");
 	}
	pthread_sigmask(SIG_BLOCK, &set2, NULL);
	int s;
	while (1) {
		if ((s = sigtimedwait(&set2, NULL, &timeout)) >  0) {
			sem_wait(&sem4);
			sig2_rec++;
			sem_post(&sem4);
			printf("sig2_rec = %d\n", sig2_rec);
		} else if (s == -1) {
			break;
		}
	}
	//pthread_sigmask(SIG_UNBLOCK, &set2, NULL);
	pthread_exit(NULL);
}

 
void* reporter() {
	pthread_exit(NULL);
}






