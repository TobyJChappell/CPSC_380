/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 2

Prompts user for input using pipes/forking and executes commands found in bin directory.

References: https://stackoverflow.com/questions/15586316/passing-arguments-to-execl
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 25
#define READ_END	0
#define WRITE_END	1

int main()
{
	char write_command[BUFFER_SIZE] = "";
	char read_command[BUFFER_SIZE];
	char q[2] = "q\n";
	char quit[5] = "quit\n";
	pid_t pid;
	int fd[2];
	while(1)
	{
		if(pipe(fd) == -1)
		{
			fprintf(stderr,"Pipe failed\n");
			return 1;
		}
		pid = fork();
		if(pid < 0)
		{
			fprintf(stderr, "Fork failed\n");
			return 1;
		}
		if(pid > 0)
		{
			close(fd[READ_END]);
			printf("osh> ");
			fgets(write_command, BUFFER_SIZE, stdin);
			if(strcmp(q, write_command) == 0 || strcmp(quit, write_command) == 0)
			{
				return 0;
			}
			write(fd[WRITE_END], write_command, strlen(write_command)+1);
			wait(NULL);
			close(fd[WRITE_END]);
		}
		else
		{
			close(fd[WRITE_END]);
			read(fd[READ_END], read_command, BUFFER_SIZE);
			if(strcmp(q, read_command) == 0 || strcmp(quit, read_command) == 0)
			{
				return 0;
			}
			execl("/bin/sh", "sh", "-c", read_command, NULL);
			close(fd[READ_END]);
		}
	}
	return 0;
}
