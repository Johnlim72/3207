#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
int main() {
	int fd = open("input.txt", O_RDONLY);
	printf("%d\n", fd);
	dup2(fd, 0);
	int a, b;
	scanf("%d", &a);
	scanf("%d", &b);
	int sum = a +b ;
	printf("%d", sum);
	printf("Welcome to the second project!\n");
	close(fd);
}
