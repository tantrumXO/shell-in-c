// Angel Walia 2017132
// Tanuj Dabas 2017118
// Prakhar Goel 2017306


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>  
#include <errno.h>   
#include <sys/wait.h> 
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_BUF_LEN 1024
#define CMD_DELIMS " \t\n"
#define MAX_TOK 64

void execute_command(char *args[]) {
	pid_t pid;
	pid = fork();

	if(pid==-1) {
		printf("fork failed!\n");
		return;
	}
	else if(pid==0) {
		//signal(SIGINT, SIG_IGN);
		if(execvp(args[0], args)==-1) {
			printf("Command not recognized!\n");
			kill(getpid(), SIGTERM);
		}
	}
	else {
		waitpid(pid, NULL, 0);
	}
}

void pipe_redir_execute_command(char *args[]) {
	pid_t pid;
	int fd1[2], fd2[2], count = 1, cur_cmd = 0;
	char *command[1024];
	bool check = false;

	int i = 0;
	while(args[i]!=NULL) {
		if(!strcmp(args[i], "|")) {
			count++;
		}
		i++;
	}

	i = 0;
	while(args[i]!=NULL && !check) {
		int j = 0;
		while(strcmp(args[i], "|")) {
			command[j] = args[i];
			i++;
			j++;
			if(args[i]==NULL) {
				check = true;
				break;
			}
		}

		i++;
		command[j]=NULL;

		if(cur_cmd%2==0) {
			pipe(fd2);
		}
		else {
			pipe(fd1);
		}

		pid = fork();

		if(pid==0) {
			if(cur_cmd == count-1) {
				if(count%2==0) {
					dup2(fd2[0], 0);
				}
				else {
					dup2(fd1[0], 0);
				}
			}
			else if(cur_cmd==0) {
				dup2(fd2[1], 1);
			}
			else {
				if(cur_cmd%2==0) {
					dup2(fd1[0], 0);
					dup2(fd2[1], 1);
				}
				else {
					dup2(fd2[0], 0);
					dup2(fd1[1], 1);
				}
			}

			// HANDLING REDIRECTION

			if(cur_cmd==0 || cur_cmd==count-1) {
				
				int counter = 0;
				int limit = -1;
				while(command[counter]!=NULL) {
					if(!strcmp(command[counter], ">")) {
						int file = open(command[counter+1], O_WRONLY|O_CREAT, 0666);
						close(1);
						dup(file);

						if(limit==-1) {
							limit = counter;
						}
					}

					else if(!strcmp(command[counter], ">>")) {
						FILE* file = fopen(command[counter+1], "a+");
						close(1);
						dup(fileno(file));

						if(limit==-1) {
							limit = counter;
						}
					}

					else if(!strcmp(command[counter], "<")) {
						int file = open(command[counter+1], O_RDONLY, 0666);
						close(0);
						dup(file);
				
						if(limit==-1) {
							limit = counter;
						}
					}

					else if(!strcmp(command[counter], "2>&1")) {
						close(2);
						dup(1);
				
						if(limit==-1) {
							limit = counter;
						}
					}

					else if(command[counter][0]=='1' && command[counter][1]=='>') {
						char *file_name = command[counter];
						file_name+=2;

						int file = open(file_name, O_WRONLY|O_CREAT, 0666);
						close(1);
						dup(file);

						if(limit==-1) {
							limit = counter;
						}	
					}

					else if(command[counter][0]=='2' && command[counter][1]=='>') {
						char *file_name = command[counter];
						file_name+=2;

						int file = open(file_name, O_WRONLY|O_CREAT, 0666);
						close(2);
						dup(file);

						if(limit==-1) {
							limit = counter;
						}	
					}

					counter++;
				}

				if(limit==-1) {
					limit = counter;
				}

				char *command_new[1024];

				int i = 0;
				while(i<limit) {
					command_new[i] = command[i];
					i++;
				}

				if(execvp(command_new[0], command_new)==-1) {
					kill(getpid(), SIGTERM);
				}

			}

			else {
				if(execvp(command[0], command)==-1) {
					kill(getpid(), SIGTERM);
				}
			}

		}

		if(cur_cmd==count-1) {
			if(count%2==0) {
				close(fd2[0]);
			}
			else {
				close(fd1[0]);
			}
		}
		else if(cur_cmd==0) {
			close(fd2[1]);
		}
		else {
			if(cur_cmd%2==0) {
				close(fd1[0]);
				close(fd2[1]);
			}
			else {
				close(fd2[0]);
				close(fd1[1]);
			}
		}

		waitpid(pid, NULL, 0);
		cur_cmd++;
	}
}

void redir_execute_command(char *args[]) {
	pid_t pid;

	pid = fork();
	int limit = -1;

	if(pid==-1) {
		printf("fork failed!\n");
		return;
	}
	else if(pid==0) {
		//signal(SIGINT, SIG_IGN);

		int counter = 0;
		while(args[counter]!=NULL) {
			if(!strcmp(args[counter], ">")) {
				int file = open(args[counter+1], O_WRONLY|O_CREAT, 0666);
				close(1);
				dup(file);

				if(limit==-1) {
					limit = counter;
				}
			}

			else if(!strcmp(args[counter], ">>")) {
				FILE* file = fopen(args[counter+1], "a+");
				close(1);
				dup(fileno(file));

				if(limit==-1) {
					limit = counter;
				}
			}

			else if(!strcmp(args[counter], "<")) {
				int file = open(args[counter+1], O_RDONLY, 0666);
				close(0);
				dup(file);
				
				if(limit==-1) {
					limit = counter;
				}
			}

			else if(!strcmp(args[counter], "2>&1")) {
				close(2);
				dup(1);
				
				if(limit==-1) {
					limit = counter;
				}
			}

			else if(args[counter][0]=='1' && args[counter][1]=='>') {
				char *file_name = args[counter];
				file_name+=2;

				int file = open(file_name, O_WRONLY|O_CREAT, 0666);
				close(1);
				dup(file);

				if(limit==-1) {
					limit = counter;
				}	
			}

			else if(args[counter][0]=='2' && args[counter][1]=='>') {
				char *file_name = args[counter];
				file_name+=2;

				int file = open(file_name, O_WRONLY|O_CREAT, 0666);
				close(2);
				dup(file);

				if(limit==-1) {
					limit = counter;
				}	
			}

			counter++;
		}

		char *command[1024];

		int i = 0;
		while(i<limit) {
			command[i] = args[i];
			i++;
		}

		if(execvp(command[0], command)==-1) {
			printf("Command not recognized!\n");
			kill(getpid(), SIGTERM);
		}
	}
	else {
		waitpid(pid, NULL, 0);
	}	
} 

void pipe_execute_command(char *args[]) {
	pid_t pid;
	int fd1[2], fd2[2], count = 1, cur_cmd = 0;
	char *command[1024];
	bool check = false;

	int i = 0;
	while(args[i]!=NULL) {
		if(!strcmp(args[i], "|")) {
			count++;
		}
		i++;
	}
	//printf("number of commands is %d", count);

	i = 0;
	while(args[i]!=NULL && !check) {
		int j = 0;
		while(strcmp(args[i], "|")) {
			command[j] = args[i];
			i++;
			j++;
			if(args[i]==NULL) {
				check = true;
				break;
			}
		}

		i++;
		command[j]=NULL;

		if(cur_cmd%2==0) {
			pipe(fd2);
		}
		else {
			pipe(fd1);
		}

		pid = fork();

		if(pid==0) {
			if(cur_cmd == count-1) {
				if(count%2==0) {
					dup2(fd2[0], 0);
				}
				else {
					dup2(fd1[0], 0);
				}
			}
			else if(cur_cmd==0) {
				dup2(fd2[1], 1);
			}
			else {
				if(cur_cmd%2==0) {
					dup2(fd1[0], 0);
					dup2(fd2[1], 1);
				}
				else {
					dup2(fd2[0], 0);
					dup2(fd1[1], 1);
				}
			}

			if(execvp(command[0], command)==-1) {
				kill(getpid(), SIGTERM);
			}
		}

		if(cur_cmd==count-1) {
			if(count%2==0) {
				close(fd2[0]);
			}
			else {
				close(fd1[0]);
			}
		}
		else if(cur_cmd==0) {
			close(fd2[1]);
		}
		else {
			if(cur_cmd%2==0) {
				close(fd1[0]);
				close(fd2[1]);
			}
			else {
				close(fd2[0]);
				close(fd1[1]);
			}
		}

		waitpid(pid, NULL, 0);
		cur_cmd++;
	}
}

int parse_command(char *args[]) {
	int counter = 0;
	bool has_pipe = false;
	bool has_redir = false;

	if(!strcmp(args[0], "exit")) {
		exit(0);
	}
	else {
		while(args[counter]!=NULL) {
			if(!strcmp(args[counter], "|")) {
				//pipe_execute_command(args);
				//return 0;
				has_pipe = true;
			}
			else if(!strcmp(args[counter], ">") || !strcmp(args[counter], ">>") || !strcmp(args[counter], "<")) {
				//redir_execute_command(args);
				//return 0;
				has_redir = true;
			}
			else if(args[counter][0]=='1' && args[counter][1]=='>') {
				//redir_execute_command(args);
				//return 0;
				has_redir = true;
			}
			else if(args[counter][0]=='2' && args[counter][1]=='>') {
				//redir_execute_command(args);
				//return 0;
				has_redir = true;
			}
			counter++;
		}

		if(has_pipe && !has_redir) {
			pipe_execute_command(args);
			return 0;
		}
		else if(!has_pipe && has_redir) {
			redir_execute_command(args);
			return 0;
		}
		else if(has_pipe && has_redir) {
			pipe_redir_execute_command(args);
			return 0;
		}
		else {
			execute_command(args);
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	pid_t pid;
	int status;
	char buffer[MAX_BUF_LEN];
	int token_count = 0;
	signal(SIGINT, SIG_IGN);

	while(1) {
		char *token[MAX_TOK];
		token_count = 0;
		memset (buffer, '\0', MAX_BUF_LEN);

		char host_name[1024];
		gethostname(host_name, 1024);
		printf("%s@%s >>>> ", getenv("LOGNAME"), host_name);
		fgets(buffer, MAX_BUF_LEN, stdin);

		token[0] = strtok(buffer, CMD_DELIMS);
		if(!strcmp(buffer, "\n")) {
			continue;
		}
		else {
			while(token[token_count]!=NULL) {
				token_count++;
				token[token_count] = strtok(NULL, CMD_DELIMS);
			}
			parse_command(token);
		}
	}

	return 0;
}