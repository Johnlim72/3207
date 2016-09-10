#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
int main() {
        printf("Hello World\n");
        printf("I am about to fork()!\n");
        pid_t pid= fork();
        if (pid == 0) {// in child process
                execlp("./rand2.out", "./rand2.out", NULL);
                printf("This will not be printed if the execlp call succeeds.\n");
        } else {// in parent process
                printf("I am in parent process\n");
                waitpid(pid, NULL, 0);
        }
        printf("End of current process\n");
}

