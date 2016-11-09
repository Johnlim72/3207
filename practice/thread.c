#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int x=0;

#define N_TIMES 500

void *optimist(void *arg);
void *pessimist(void *arg);

int main(int argc, char **argv) {
	pthread_t A, B;

	pthread_create(&A, NULL, optimist, NULL);
 	pthread_create(&B, NULL, pessimist, NULL);
 	pthread_join(A, NULL);
 	pthread_join(B, NULL);
	printf("done. x=%d\n", x);
 
	return 0;
}

void *optimist(void *arg) {
	int i;
	for (i=0; i< N_TIMES; i++) {
		x++;
	}
}

void *pessimist(void *arg) {
	int i;
	for (i = 0; i < N_TIMES; i++) {
		x--;
	}
}
