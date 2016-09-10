#include <stdio.h>
#include <sys/time.h>

int main() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("Current time (seconds): %d\n", tv.tv_sec);
	printf("Current time (microseconds): %d\n", tv.tv_usec);
}
