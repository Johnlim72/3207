#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
int main() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	sleep(10);
	usleep(3000);
	struct timeval tv2;
	gettimeofday(&tv2, NULL);
	printf("Previous time (seconds): %d\n", tv.tv_sec);
	printf("Previous time (microseconds): %d\n", tv.tv_usec);
	printf("End time (seconds): %d\n", tv2.tv_sec);
	printf("End time (microseconds): %d\n", tv2.tv_usec);
}
