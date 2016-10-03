#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

char **parse(char *str);
int exec_norm(char *str);
int exec_bg(char* str);
int exec_redir(char *str);
int exec_pipe(char *str);
int check_builtins(char** args, char *str);
void prompt(char *str);

int main() {
	char str[1024];
	prompt(str);
	while (1) {
		//Quits the shell.
		if (strcmp(str, "quit") == 0) {
			exit(0);
		}		
		
		int i =0;
		while (str[i] != '\0') {
			if (str[i+1] == '\0') {
				printf("normal exec\n");
				if (exec_norm(str) != 0) {
					printf("error in exec_norm\n");
				}
				break;
			} else if (str[strlen(str)-1] == '&') {
				printf("background exec\n");
				if (exec_bg(str) != 0) {
					printf("error in exec_bg\n");
				}
				break;
			} else if ((str[i] == '>') || (str[i] == '<')) {
				printf("redir exec\n");
				if (exec_redir(str) != 0) {
					printf("error in exec_redir\n");
				}
				break;
			} else if (str[i] == '|') {
				printf("pipe exec\n");
				 if (exec_pipe(str) != 0) {
					printf("error in exec_pipe\n");
				} 
				break;		
			}
			i++;
		}
	prompt(str);
	}	
	return 0;
}

void prompt(char *str) {
	char currentDir[1024];
	getcwd(currentDir, sizeof(currentDir));
	printf("%s>", currentDir);
	gets(str);
}

char **parse(char *str) {
	char **args = malloc(1024 * sizeof(char*));
	int i=0;
	char *arg;

	arg = strtok(str, "  \t\n");
	while (arg != NULL) {
		args[i] = arg;
		arg = strtok(NULL, "  \t\n");
		i++;	}
	args[i] = NULL;
	return args;
}  

int exec_norm(char *str) {
	char **args = parse(str);
	char* path = args[0];
	char *parameters[20];
	
	int i=0;
	while (args[i] != NULL) {
		parameters[i] = args[i];
		i++;
	}
	parameters[i] = NULL;
	
	if (check_builtins(args,str) == 0) {
		return 0;
	}
	pid_t pid = fork();
	if (pid == 0) {
		if (execvp(path, parameters) != 0) {
			printf("error in executing\n");
			exit(0); // so you dont need mult quits to quit
		}
	} else {
		waitpid(pid, NULL, 0);
	}
	return 0;
}

int exec_bg(char *str) {
	char **args = parse(str);
        char* path = args[0];
        char *parameters[20];

        int i=0;
        while (args[i][0] != '&' ) {
                parameters[i] = args[i];
		i++;
        }
        parameters[i] = NULL;

	if (check_builtins(args, str) == 0) {
		return 0;
	}
	
        pid_t pid = fork();
        if (pid == 0) {
               	if (execvp(path, parameters) != 0) {
			printf("error in executing in background\n");
			exit(0);
		}
        } 
        return 0;
}

int exec_redir(char *str) {
	
	char** args = parse(str);
	char* path = args[0];
	char *parameters[20];
	
	int i =0;
	int k = 0;
	int input_count = 0;
	int output_count = 0;

	while ((args[i][0] != '<') && (args[i][0] != '>')) { 
		parameters[i] = args[i];
		i++; 
	}
	parameters[i] = NULL;

	int l_count = 0; //letter count
	while (str[l_count] != '>') {
		l_count++;
	}	

	while (args[k] != NULL) {
		if (args[k][0] == '<') {
			input_count++;
		} else if (args[k][0] == '>') {
			output_count++;
			if ((str[l_count+1]) && (args[k][1] == '>')) {
				output_count++;
			}
		}
		k++;
	}

	printf("input: %d, output: %d\n", input_count, output_count);
	 if (input_count == output_count == 1) {     
		pid_t pid = fork();
		if (pid == 0) {
			int fd_in = open(args[i+1], O_RDONLY);
			int fd_out = open(args[i+3], O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
			dup2(fd_in, 0);
			dup2(fd_out, 1);
			if (execvp(path, parameters) != 0) {
				printf("error in executing using redirection\ninput_count: 1, output_count: 1\n");
				exit(0);
			}
			close(fd_in);
			close(fd_out);
		} else {
			waitpid(pid, NULL, 0);
		}
		return 0;
	 } else if ((output_count == 2) && (input_count == 1)) {
		pid_t pid = fork();	
		if (pid == 0) {
			int fd_in = open(args[i+1], O_RDONLY);
			int fd_out = open(args[1+3], O_WRONLY|O_CREAT|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO);
			dup2(fd_in, 0);
			dup2(fd_out, 1);
			if (execvp(path, parameters) != 0) {
                                printf("error in executing using redirection\ninput_count: 2, output_count: 1\n");
				exit(0);
			}
			close(fd_in);
			close(fd_out);
		} else {
			waitpid(pid, NULL, 0);
		}
	} else if ((output_count == 2) && (input_count==0)) {
		pid_t pid = fork();
		if (pid == 0) {
			int fd = open(args[i+1], O_WRONLY|O_CREAT|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO);
			dup2(fd, 1);
			if (check_builtins(args, str) == 0) {
				
			} else if (execvp(path, parameters) != 0) {
                                printf("error in executing using redirection\ninput_count: 2, output_count: 0\n");	
				exit(0);
			}
			close(fd);
			exit(0);
		} else {
			waitpid(pid, NULL, 0);
		}
	} else if (args[i][0] == '<') { 
		pid_t pid= fork();
		if (pid == 0) {
			int fd = open(args[i+1], O_RDONLY); // opens text file for input
			dup2(fd, 0);
			if (execvp(path, parameters) != 0) {
                                printf("error in executing using redirection\ninput_count: 1, output_count: 0\n");
				exit(0);
			}
			close(fd);
		} else {
			waitpid(pid, NULL, 0);
		}		
         } else if (args[i][0] == '>') { //execution when there is only output redirection
		pid_t pid = fork();
		if (pid == 0) {
			int fd = open(args[i+1], O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
			dup2(fd, 1);
			if (check_builtins(args, str) == 0) {
								
			}
			 if (execvp(path, parameters) != 0) {
                                printf("error in executing using redirection\ninput_count: 0, output_count: 1\n");
				exit(0);	
			}
			close(fd);
			exit(0);
		} else {
			waitpid(pid, NULL, 0);
		}
	 }
	 return 0;
}

int exec_pipe(char* str) {
	char** args = parse(str);
	char *path1 = args[0];
	char* parameters1[20];
	char* parameters2[20];
	
	int i = 0;
	while (args[i][0] != '|') { 
		parameters1[i] = args[i];
		i++;
	}
	parameters1[i] = NULL;
	i++;

	int j = 0;
	char *path2 = args[i];
	while (args[i] != NULL) {
		parameters2[j] = args[i];
		j++;
		i++;
	}
	parameters2[j] = NULL;
	
	int fd[2];
	pipe(fd);
	int pid = fork();
	if (pid == 0) {
		dup2(fd[0], 0);
		close(fd[1]);
		if (execvp(path2, parameters2) != 0) {
			printf("error in child, executing with pipe\n");
			exit(0);
		}
	} else {
		int pid2 = fork();
		if (pid2 == 0) {
			dup2(fd[1], 1);
			close(fd[0]);
			if (execvp(path1, parameters1) != 0) {
				printf("error in parent, executing with pipe\n");
				exit(0);
			}
		} else {
			waitpid(pid2,NULL,0);
		}
		waitpid(pid, NULL, 0);
	}
	return 0;
}


int check_builtins(char **args, char* str) {
	printf("check builtin\n");
	if (args[0] == NULL) {
		return 0;
	}
	if (strcmp(args[0], "cd") == 0) { //cd : change directory
		if(args[1] == NULL) {
			printf("error in cd exec: no arg provided.\n");
			return 0;
		} else {
			if (chdir(args[1]) != 0) {
				printf("error in cd exec\n");
			}
		}
		return 0;	
	} else if (strcmp(args[0], "clr") == 0) {
		system("clear");
		return 0;
	} else if (strcmp(args[0], "dir") == 0) {
		DIR *dp;
		struct dirent *list;
		char path[1024];
		strcpy(path, args[1]);
		if (args[1] == NULL) {
			printf("error in dir exec: no path provided.\n");
			return 0;
		} else {
			if ((dp = opendir(path)) == NULL) {
				printf("opendir %s error", path);
			}
			while ((list = readdir(dp))) {
				printf("%s\n", list -> d_name);
			}
			closedir(dp);
		}
		return 0;
	} else if (strcmp(args[0], "environ") == 0) {
		system("printenv");	
		return 0;
	} else if (strcmp(args[0], "echo") ==0) {
		char echo_string[1024];
		int i=0;
		while (args[i] != NULL) {
			strcat(echo_string,args[i]);
			strcat(echo_string," ");
			i++;
		}
		system(echo_string);
		strcpy(echo_string, "");
		return 0;
	} else if (strcmp(args[0], "pause") == 0) {
		char c;
		printf("Operation paused. Press enter to continue.\n");
		while (c = getchar() != '\n') {
		}
		return 0;
	}	
	return -1;
}

