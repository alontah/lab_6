#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  int pipefd[2];
  int pid;

  if(pipe(pipefd) == -1){
    perror("pipe() failed");
    exit(1);
  }

  if((pid = fork())){
    //parent process
    close(pipefd[1]);
    char  parent_msg[100];
    for(int i = 0; i < 100 && read(pipefd[0], &parent_msg[i], 1) > 0; i++); //reading 1 char at a time
    printf("child procces wrote: %s\n", parent_msg);
    close(pipefd[0]);
  } else {
    //child process
    close(pipefd[0]);
    char * msg = "pasha is geh";
    write(pipefd[1], msg, strlen(msg));
    close(pipefd[1]);
    exit(0); //child process done
  }

  return 0;
}
