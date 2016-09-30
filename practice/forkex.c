#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main (void) {
pid_t pid;
int i;
for (i=0; i<3; i++) {
if ((pid=fork()) <0) {
//fprintf(stderr, "fork error: %s\n", strerror(errno));
} else if (pid == 0) {
printf("child: %d\n", i);
} else {
printf("parent: %d\n", i);
}
}
return 0;
}
