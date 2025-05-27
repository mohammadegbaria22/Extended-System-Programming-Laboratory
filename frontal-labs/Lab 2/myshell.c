#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "LineParser.h"
#include <signal.h>

int toWait = 1;

void handleProcessCommand(cmdLine *pCmdLine) {
    if (pCmdLine->argCount < 2) {
        fprintf(stderr, "Error: Process ID missing\n");
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);

    if (strcmp(pCmdLine->arguments[0], "halt") == 0) {
        if (kill(pid, SIGSTOP) == -1)
            perror("Error stopping process");
        else
            printf("Process %d stopped\n", pid);
    } else if (strcmp(pCmdLine->arguments[0], "wakeup") == 0) {
        if (kill(pid, SIGCONT) == -1)
            perror("Error waking process");
        else
            printf("Process %d continued\n", pid);
    } else if (strcmp(pCmdLine->arguments[0], "ice") == 0) {
        if (kill(pid, SIGKILL) == -1)
            perror("Error killing process");
        else
            printf("Process %d killed\n", pid);
    }
}

void execute(cmdLine *pCmdLine, int debug) {
    // Handle 'cd' command
    if (strcmp(pCmdLine->arguments[0], "cd") == 0) {
        if (pCmdLine->argCount < 2)
            fprintf(stderr, "cd: missing argument\n");
        else if (chdir(pCmdLine->arguments[1]) != 0)
            perror("cd failed");
        return;
    }

    // Handle signal commands
    if (strcmp(pCmdLine->arguments[0], "halt") == 0 ||
        strcmp(pCmdLine->arguments[0], "wakeup") == 0 ||
        strcmp(pCmdLine->arguments[0], "ice") == 0) {
        handleProcessCommand(pCmdLine);
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // CHILD

        // Input redirection
        if (pCmdLine->inputRedirect) {
            int fd = open(pCmdLine->inputRedirect, O_RDONLY);
            if (fd == -1) {
                perror("Error opening input file");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Output redirection
        if (pCmdLine->outputRedirect) {
            int fd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("Error opening output file");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (debug) {
            fprintf(stderr, "Child executing: %s\n", pCmdLine->arguments[0]);
        }

        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("execvp failed");
        exit(1);
    } else {
        // PARENT
        if (debug) {
            fprintf(stderr, "Parent PID: %d, Child PID: %d\n", getpid(), pid);
        }

        if (pCmdLine->blocking) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Process %d exited with code %d\n", pid, WEXITSTATUS(status));
            }
        } else if (debug) {
            printf("Running in background, not waiting for PID %d\n", pid);
        }
    }
}

int main(int argc, char **argv) {
    int debug = 0;

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    }

    while (1) {
        char cwd[2048];
        getcwd(cwd, sizeof(cwd));
        printf("%s > ", cwd);
        fflush(stdout);

        char input[2048];
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        // Strip newline
        input[strcspn(input, "\n")] = 0;

        // Quit command
        if (strcmp(input, "quit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        cmdLine *parsedInput = parseCmdLines(input);
        if (!parsedInput) continue;

        execute(parsedInput, debug);

        freeCmdLines(parsedInput);
    }

    return 0;
}
