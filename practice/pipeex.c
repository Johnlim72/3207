#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#define BUF_SIZE 64
#define READ 0
#define WRITE 1
int main(int argc, char **argv) {
int fd[2];
int bytes_read;
pid_t pid;
char buf[BUF_SIZE];
char *who="I'm the child: ";
if (pipe(fd)==-1) {
//fprintf(stderr, "pipe error: %s\n", strerror(errno));
return 0;
}
if ((pid=fork())==-1) {
//fprintf(stderr, "fork error: %s\n", strerror(errno));
return 0;
} else if (pid==0) {
close(fd[WRITE]);
dup2(fd[READ], STDIN_FILENO);
if ((bytes_read=read(STDIN_FILENO, buf, BUF_SIZE))>0) {
write(STDOUT_FILENO, who, strlen(who));
write(STDOUT_FILENO, buf, bytes_read);
}
} else {
close(fd[READ]);
dup2(fd[WRITE], STDOUT_FILENO);
printf("Who lives in a pineapple under the sea?\n");
}
return 0;
}
