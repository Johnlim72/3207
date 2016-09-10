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
                printf("I am in child process\n");
                sleep(10);
                exit(0);
        } else {// in parent process
                printf("I am in parent process\n");
                waitpid(pid, NULL, 0);
        }
        printf("End of current process\n");
}

