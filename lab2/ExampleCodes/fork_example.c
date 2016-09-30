#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
pid_t my_pid;
int i=0;
while (i<3) {
printf("before fork: i=%d\n", i);
i++;
}
if (fork()==-1) {
printf("fork error: %s\n", "sup");
return 0;
}
my_pid=getpid();
while (i<6) {
printf("my pid is %d: i=%d\n", my_pid, i);
i++;
}
return 0;
}
