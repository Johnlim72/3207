#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>

void prompt(char *str);
void check_exec(char *str);
char **parse(char *str);
int exec_norm(char *str);
int exec_bg(char* str);
int exec_redir(char *str);
int exec_pipe(char *str);
int check_builtins(char** args, char *str);

int main(int args, char* argv[]) {
	//Checks if there is a batchfile with list of commands.
	if (argv[1] != NULL) {
		if (strcmp(argv[1], "batchfile") == 0) {
			FILE *fp = fopen(argv[1], "r"); //Opens batchfile.
			if (fp == NULL) {
				printf("error opening batchfile.\n");
				return 0;	
			}
			char str[1024];
			while (fgets(str, 1024, fp)) { //Goes through each line of the batchfile.
				printf("str: %s", str);
				if (str[strlen(str)-1] == '\n') { //In order to execute commands (need null character at end of command). 
					str[strlen(str)-1] = '\0';
				}
				if (strcmp(str, "quit") == 0) { //Quits the shell.
					exit(0);
				} 
				check_exec(str); //Checks which kind of execution will be run, then executes command.
			}
			fclose(fp);
		}	
	} else { //No batchfile; Execute commands by input from user.
		char str[1024];
		while (1) {
			prompt(str); //Prints prompt.

			if (strcmp(str, "quit") == 0) { //Quits the shell.
				exit(0);
			}		
			check_exec(str); //Checks which kind of execution will be run, then executes command.
		}	
		return 0;
	}
}

void prompt(char *str) { //Prints prompt.
	char currentDir[1024];
	getcwd(currentDir, sizeof(currentDir)); //Gets current directory.
	printf("%s>", currentDir);
	gets(str); //Gets input from user.
}

void check_exec(char *str) { //Checks which kind of execution will be run, then executes command.
	int i =0;
	while (str[i] != '\0') {
		if (str[i+1] == '\0') { //Executes command (no &, <, >, >>, | arguments);
			if (exec_norm(str) != 0) {
				printf("error in exec_norm\n");
			}
			break;
		} else if (str[strlen(str)-1] == '&') { //Executes command in background.
			if (exec_bg(str) != 0) {
				printf("error in exec_bg\n");
			}
			break;
		} else if ((str[i] == '>') || (str[i] == '<')) { //Executes command with redirection.
			if (exec_redir(str) != 0) {
				printf("error in exec_redir\n");
			}		
			break;
		} else if (str[i] == '|') { //Execute commands with pipe.
			if (exec_pipe(str) != 0) {
				printf("error in exec_pipe\n");
			}	 
			break;		
		}
		i++;
	}
}

char **parse(char *str) { //Returns array of string of the string seperated by whitespace/tab character/new line character.
	char **args = malloc(1024 * sizeof(char*));
	int i=0;
	char *arg;
	arg = strtok(str, "  \t\n");
	while ((arg != NULL) && (strcmp(arg, "&")) != 0) {
		args[i] = arg;
		arg = strtok(NULL, "  \t\n");
		i++;	
	}
	args[i] = NULL;
	return args;
}
  
int exec_norm(char *str) { //Executes command (no &, <, >, >>, | arguments);
	char **args = parse(str);
	if (check_builtins(args, str) == 0) {
		return 0;
	}
	int pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) != 0) {
			printf("error in executing command\n");
			exit(0); // so you dont need mult quits to quit
		}
	} else {
		wait(NULL);
	}
	return 0;
}

int exec_bg(char *str) { //Executes command in background.
	char **args = parse(str);	
	if (check_builtins(args, str) == 0) {
		return 0;
	}	
        int pid = fork();
        if (pid == 0) {
               	if (execvp(args[0], args) != 0) {
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
		dup2(fd[1], 1);
		close(fd[0]);
		if (execvp(path1, parameters1) != 0) {
			printf("error in parent, executing with pipe\n");
			exit(0);
		}		
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

