#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 100
void *func(void *arg);

int main(void) {
	pthread_t thread_ids[NUM_THREADS];
	int i;
	
	for (i=0; i< NUM_THREADS; i++) {
		pthread_create(&thread_ids[i], NULL, func, &i);
	}
	for (i =0; i< NUM_THREADS; i++) {
		pthread_join(thread_ids[i], NULL);
	return 0;
}
}

void *func(void *arg) {
	int id = *(int*) (arg);
	printf("ID =%d\n", id);
	return NULL;
}
