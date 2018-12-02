// Создание процесса-сироты

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define REPEAT_TIMES 5

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
        sleep(1);
        exit(0);
    }
    else if(childPID1 == 0)
    {
        // Child 1
        for(int i = 0; i < REPEAT_TIMES; i++)
        {
            printf("Child 1. PID = %d, ParentPID = %d\n", getpid(), getppid());
            sleep(1);
        }
        exit(0);
    }
    else
    {
        // Child 2
        for(int i = 0; i < REPEAT_TIMES; i++)
        {
            printf("Child 2. PID = %d, ParentPID = %d\n", getpid(), getppid());
            sleep(1);
        }
        exit(0);
    }

    return 0;

}