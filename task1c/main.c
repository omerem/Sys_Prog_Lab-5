#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "LineParser.h"
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <signal.h>

void execute(cmdLine *pCmdLine);
void pwd();

void pwd()
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    printf("%s$", cwd);
}


void execute(cmdLine *pCmdLine)
{
    if (execvp((pCmdLine->arguments[0]),  pCmdLine->arguments ) < 0)
    {
        perror("Error: ");
        exit(errno);
    }
    fclose(stdin);
}


int main(int argc, char **argv)
{
    int debug = 0;
    int i=0;
    int status;
    int cd;
    for(i=0;i<argc;i++)
    {
        if (strcmp(argv[i],"-d")==0)
        {
            debug = 1;
        }
    }

    

    printf("Starting the program\n");
    char input[2048];
    cmdLine * parsedCLines;
    int pid;

    while (1) {
        pwd();
        fgets(input, 2048, stdin);
        if (strcmp(input, "quit\n")==0)
        {
            exit(0);
        }
        parsedCLines = parseCmdLines(input);

        pid = fork();
        if (pid < 0) {
            freeCmdLines(parsedCLines);
            exit(1);
        }

        if (debug) {
            fprintf(stderr, "PID = %d", pid);
            fprintf(stderr, "\nExecuting Command = %s", parsedCLines->arguments[0]);
            fprintf(stderr, "\n");
        }
        if (strcmp(parsedCLines->arguments[0], "cd") == 0)
        {
            cd = 1;
        }
        else
        {
            cd = 0;
        }
        if((cd == 1) && (pid != 0)) // quarter 1
        {
            status = chdir(parsedCLines->arguments[1]);
            if (status == -1) // cd failed
            {
                perror("Error: ");
                freeCmdLines(parsedCLines);
                kill(-1, SIGKILL);
                exit(errno);
            }
        }
        else if ((cd == 0) && (pid != 0)) // quarter 2
        {
            waitpid(pid, &status,!parsedCLines->blocking);
        }
        else if ((cd == 1) && (pid == 0)) // quarter 3
        {
            freeCmdLines(parsedCLines);
            _exit(0);
        }
        else if ((cd  == 0) && (pid == 0)) // qurter 4
        {
            execute(parsedCLines);
            freeCmdLines(parsedCLines);
            _exit(errno);
        }

        freeCmdLines(parsedCLines);
    }

    return 0;
}
