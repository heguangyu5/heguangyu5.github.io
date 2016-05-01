#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 21

int main(int argc, char *argv[])
{
    printf("mypid = %d\n", getpid());

    char buf[500];
    char *p;
    char *args[MAX_ARGS];
    uint8_t idx;
    pid_t pid;

    while (1) {
        printf("\n\ninput the program you want to run:\n");
        if (fgets(buf, 500, stdin) == NULL) {
            break;
        }

        p = buf;
        while (*p == ' ' || *p == '\t') p++;

        idx = 0;
        while (idx < MAX_ARGS && *p) {
            args[idx] = p;
            idx++;
            while (*p && *p != ' ' && *p != '\t' && *p != '\n') p++;
            if (*p == '\n') {
                *p = 0;
                break;
            }
            while (*p && (*p == ' ' || *p == '\t')) {
                *p = 0;
                p++;
            }
            if (*p == '\n') {
                *p = 0;
                break;
            }
        }
        args[idx] = NULL;

        printf("GOT args:\n");
        idx = 0;
        while (args[idx]) {
            printf("args[%d] = %s\n", idx, args[idx]);
            idx++;
        }

        printf("Now fork then exec\n");
        pid = fork();
        if (pid > 0) {
            printf("fork a child (pid = %d) for %s\n", pid, args[0]);
            // wait for child terminate
            if (waitpid(pid, NULL, 0) == pid) {
                printf("child %d terminated\n", pid);
            }
        } else if (pid == 0) {
            printf("will run %s\n", args[0]);
            if (execv(args[0], args) == -1) {
                printf("run %s failed\n", args[0]);
                perror("execv");
                exit(1);
            }
        } else {
            perror("fork");
        }
    }

    printf("EXIT\n");

    return 0;
}
