#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 100
#define NUM_TIMES 500

void *func(void *arg);

int main(void) {
	pthread_t thread_ids[NUM_THREADS];
	int i;
	
	for (i=0; i< NUM_THREADS; i++) {
		pthread_create(&thread_ids[i], NULL, func, &i);
	}
	for (i =0; i< NUM_THREADS; i++) {
		pthread_join(thread_ids[i], NULL);
	}
	return 0;
}

void *func(void *arg) {
	int i;
	int id = (int)(arg);
	static int x = 0;
	
	for (i =0; i <NUM_TIMES; i++) {
		x++;
	}

	printf("ID =%d, x=%d\n", id, x);
	return NULL;
}
