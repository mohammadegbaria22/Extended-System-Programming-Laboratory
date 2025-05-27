#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128

int main(int argc, char **argv){
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    if(pipe(pipefd) == -1){
        perror("pipe failed!");
        exit(1);
    }

    pid = fork();

    if(pid == -1){
        perror("fork failed!");
        exit(1);
    }

    if(pid == 0){
        //| | ~~~~~~~~~~~~~~~~~~~~~~~
        //V V the child process logic
        close(pipefd[1]);
        ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer)-1);
        if(bytesRead == -1){
            perror("could not read!");
            exit(1);
        }

        buffer[bytesRead] = '\0'; // end the message with the null char
        printf("Child received message: %s\n", buffer);
        close(pipefd[0]);
        exit(0);
    } else {
        //| | ~~~~~~~~~~~~~~~~~~~~~~~
        //V V the parent process logic
        close(pipefd[0]);
        const char *message = argv[1];
        if (write(pipefd[1], message, strlen(message)) == -1) {
            perror("write failed");
            exit(1);
        }
        close(pipefd[1]);
        wait(NULL); // Wait for child to finish
    }

    return 0;
}