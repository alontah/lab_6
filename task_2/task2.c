#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "LineParser.h"

#define STDIN 0
#define STDOUT 1
#define BUFFER_SIZE 2048

int debug = 0;

void change_output(cmdLine * cmd_line){
	if(cmd_line->outputRedirect){
		close(STDOUT);
		fopen(cmd_line->outputRedirect, "w"); //now instead of stdout default output (1) is inputRedirect file
	}
}

void change_input(cmdLine * cmd_line){
	if(cmd_line->inputRedirect){
		close(STDIN);
		fopen(cmd_line->inputRedirect, "r"); //now instead of stdin default input (0) is inputRedirect file
	}
}


int execute_single_pipe(cmdLine * cmd_line){
	int pipefd[2];
	int pid1, pid2, status1, status2;

	if(pipe(pipefd) == -1){
		perror("pipefd error");
		exit(1);
	}

	if((pid1 = fork())){
		//parent process
		close(pipefd[1]);
		if((pid2 = fork())){
			//parent process
			close(pipefd[0]);
			if(cmd_line -> blocking){
				waitpid(pid1, &status1, WUNTRACED);
			}
			if(cmd_line->next->blocking){
				waitpid(pid2, &status2, WUNTRACED);
			}
		} else {
			//second child process
			close(0);
    	int dup_pid = dup(pipefd[0]);
      close(pipefd[0]);

			cmdLine * second_cmd = cmd_line->next;
			change_output(second_cmd);
			execvp(second_cmd->arguments[0], second_cmd->arguments);
			perror("second cmd error");
	    exit(1);
		}
	} else {
		//first child process
		change_input(cmd_line);
		close(1);
    dup(pipefd[1]);
    close(pipefd[1]);
		execvp(cmd_line->arguments[0], (cmd_line->arguments));
    perror("cmd error");
    exit(1);
	}
}


int execute_single_cmd(cmdLine *cmd_line){
	if(cmd_line){
		int pid = fork();
		int status;

		if(pid){
			//parent process
			if(debug){
				fprintf(stderr, "PID num: %d, Executing command: %s\n", pid, cmd_line->arguments[0]);
			};
			waitpid(pid, &status, (1 - cmd_line->blocking) | WUNTRACED);
			return status;
		} else {
			//child process
			change_input(cmd_line);
			change_output(cmd_line);
			if(execvp(cmd_line->arguments[0], cmd_line->arguments) < 0){
				perror("error\n");
				_exit(1);
			}
			_exit(0);
		}
	}
}

int execute(cmdLine *cmd_line){
	if(cmd_line && cmd_line->next){
		return execute_single_pipe(cmd_line);
	} else {
		return execute_single_cmd(cmd_line);
	}
}

int main(int argc, char **argv){
	char curr_path_buff[BUFFER_SIZE];
	char user_buff[BUFFER_SIZE];
	printf("Starting the program\n");

	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "-d") == 0){
			debug = 1;
		}
	}

	while(1) {
		getcwd(curr_path_buff, BUFFER_SIZE);
		printf("%s\n",curr_path_buff);
		fgets(user_buff, BUFFER_SIZE, stdin);
		cmdLine *cmd_line = parseCmdLines(user_buff);
		if(strcmp(cmd_line->arguments[0], "quit") == 0){
			freeCmdLines(cmd_line);
			break;
		} else if(strcmp(cmd_line->arguments[0], "cd") == 0){
			if(cmd_line -> argCount < 2 && debug){
				fprintf(stderr, "no path entered\n");
			} else if(chdir(cmd_line->arguments[1]) < 0 && debug) {
				fprintf(stderr, "path: %s not found\n", cmd_line->arguments[1]);
			}
			freeCmdLines(cmd_line);
		} else {
			execute(cmd_line);
			freeCmdLines(cmd_line);
		}
	}

	return 0;
}
