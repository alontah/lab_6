#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "LineParser.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define BUFFER_SIZE 100

int main(int argc, char const *argv[]) {
  int debug = 0;
  for(int i = 0; i < argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      debug = 1;
    }
  }

  int pipefd[2];
  int pid1, pid2, status1, status2;

  if(pipe(pipefd) == -1){
    perror("pipe() failed");
    exit(1);
  }

  if(debug){
    fprintf(stderr, "parent_procces>forking\n");
  }
  if((pid1 = fork())){
    //parent procces
    if(debug){
      fprintf(stderr, "parent_procces>created process with id: %d\n", pid1);
      fprintf(stderr, "parent_procces>closing the write end of the pipe\n");
    }
    close(pipefd[1]);

    if((pid2 = fork())){
      //parent procces
      if(debug){
        fprintf(stderr, "parent_procces>created process with id: %d\n", pid2);
        fprintf(stderr, "parent_procces>closing the read end of the pipe\n");
      }
      close(pipefd[0]);

      if(debug){
        fprintf(stderr, "parent_procces>waiting for child proccesses to terminate\n");
      }
      if(waitpid(pid1, &status1, 0) < 0){
        perror("error waitpid pid1");
      }
      if(waitpid(pid2, &status2, 0) < 0){
        perror("error waitpid pid2");
      }
      if(debug){
        fprintf(stderr, "parent_procces>exiting\n");
      }
    } else {
      //child procces two
      if(debug){
        fprintf(stderr, "child2>redirecting stdin to read end of the pipe\n");
      }

      close(STDIN);
      dup(pipefd[0]);
      close(pipefd[0]);
      cmdLine * second_cmd = parseCmdLines("tail -n 2");
      if(debug){
        fprintf(stderr, "child2>going to execute cmd %s\n", second_cmd->arguments[0]);
      }
      execvp(second_cmd->arguments[0], second_cmd->arguments);
      freeCmdLines(second_cmd);

      exit(0);
    }
  } else {
    //child process one
    if(debug){
      fprintf(stderr, "child1>redirecting stdout to write end of the pipe\n");
    }
    close(STDOUT);
    dup(pipefd[1]);
    close(pipefd[1]);
    cmdLine * first_cmd = parseCmdLines("ls -l");
    if(debug){
      fprintf(stderr, "child1>going to execute cmd %s\n", first_cmd->arguments[0]);
    }
    execvp(first_cmd->arguments[0], first_cmd->arguments);
    freeCmdLines(first_cmd);
    exit(0); //child process done
  }

  return 0;
}
