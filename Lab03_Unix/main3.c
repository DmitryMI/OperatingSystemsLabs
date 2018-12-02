// child with exec

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define REPEAT_TIMES 3


void analise_status(int wait_status)
{
    int exit_status = WEXITSTATUS(wait_status);
    if(WIFEXITED(wait_status))
        printf("Parent. Child exited normally with exit status %d\n", exit_status);
}

void analise_status_mult(int wait_status, int child_pid)
{
    int exit_status = WEXITSTATUS(wait_status);
    if(WIFEXITED(wait_status))
        printf("Parent. Child %d exited normally with exit status %d\n", child_pid, exit_status);
}


int main(void)
{
    int childPID1, childPID2;

    childPID1 = fork();
    

    if(childPID1 == -1)
    {
        perror("Fork 1 failed!");
        exit(-1);
    }

    if(childPID1 != 0)
    {
        childPID2 = fork();
    
        if(childPID2 == -1)
        {
            perror("Fork 2 failed!");
            exit(-1);
        }
    }

    if(childPID1 != 0 && childPID2 != 0)
    {
        // Parent
        printf("Parent. PID = %d, childPID1 = %d, childPID2 = %d\n", getpid(), childPID1, childPID2);

        int status;
        //wait(&status);
        waitpid(childPID1, &status, WUNTRACED);

        analise_status_mult(status, childPID1);

        waitpid(childPID2, &status, WUNTRACED);

        analise_status_mult(status, childPID2);

        exit(0);
    }
    else if(childPID1 == 0)
    {
        // Child 1

        int exec_result = execl("/bin/ls", "ls", "-l", NULL);

        if(exec_result == -1)
        {
            perror("exec failed!");
        }
    }
    else
    {
        // Child 2
        sleep(1);

        int exec_result = execl("/bin/pwd", "pwd", NULL);

        if(exec_result == -1)
        {
            perror("exec failed!");
        }
    }

    return 0;
}