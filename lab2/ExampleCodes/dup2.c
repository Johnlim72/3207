#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main() {
	int fd = open("output_destination.txt", O_CREAT|O_WRONLY, S_IRWXU);
	printf("%d\n", fd);
	dup2(fd, 1);
	printf("Welcome to the second project!\n");
	close(fd);
}
