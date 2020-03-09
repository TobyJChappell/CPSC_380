/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 2

Prompts user for input using pipes/forking and executes commands found in bin.

References: https://stackoverflow.com/questions/15586316/passing-arguments-to-execl
*/

#include <stdio.h>
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
	char quit[BUFFER_SIZE] = "q";
	pid_t pid;
	int fd[2];
	while(strcmp(write_command,"q") != 0)
	{
		if (pipe(fd) == -1)
		{
			fprintf(stderr,"Pipe failed");
			return 1;
		}
		pid = fork();
		if (pid < 0)
		{
			fprintf(stderr, "Fork failed");
			return 1;
		}
		if (pid > 0)
		{
			close(fd[READ_END]);
			printf("osh > ");
			fgets(write_command, BUFFER_SIZE, stdin);
			printf("\n");
			write(fd[WRITE_END], write_command, strlen(write_command)+1);
			close(fd[WRITE_END]);
		}
		else
		{
			close(fd[WRITE_END]);
			read(fd[READ_END], read_command, BUFFER_SIZE);
			execl("/bin/sh", "sh", "-c", read_command, NULL);
			close(fd[READ_END]);
		}
	}
	return 0;
}
