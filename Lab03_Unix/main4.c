#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

// Pipe

void analise_status_mult(int wait_status, int child_pid)
{
    int exit_status = WEXITSTATUS(wait_status);
    if(WIFEXITED(wait_status))
        printf("Parent. Child %d exited normally with exit status %d\n", child_pid, exit_status);
}

int main(void)
{
	int childPid1, childPid2;
		
	// fd[0] is open for reading
	// fd[1] is open for writing
	int fd[2];
	char read_buffer[100];
	
	if(pipe(fd) == -1)
	{
		perror("Pipe error");
		exit(1);
	}
	
	childPid1 = fork();
	if(childPid1 == -1)
	{
		perror("Fork 1 failed!");
		exit(1);
	}
	
	if(childPid1 != 0)
	{
		childPid2 = fork();
		if(childPid2 == -1)
			perror("Fork 2 failed!");
		exit(1);
	}
	
	if(childPid1 != 0 && childPid2 != 0)
	{				
		// Parent
		close(fd[1]);
		
		for(int i = 0; i < 2; i++)
		{
			read(fd[0], read_buffer, 100);
			
			printf("Parent %d read: %s\n", getpid(), read_buffer);
		}
		
		close(fd[0]);
		
		int status;
		
        waitpid(childPid1, &status, WUNTRACED);
        analise_status_mult(status, childPid1);
        waitpid(childPid2, &status, WUNTRACED);
        analise_status_mult(status, childPid2);		
        
		printf("Parent %d exit\n", getpid());
			
		exit(0);
	}
	else
	{
		// Child 1 and Child 2
		close(fd[0]);
		char buffer[100];
		sprintf(buffer, "Hello from child %d\n", getpid());
		write(fd[1], buffer, 100);
		
		close(fd[1]);
		
		printf("Child %d exit\n", getpid());
		exit(0);
	}
	
	
	
}
