/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 1

Concatenates one file to another using system calls.

References: https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include<fcntl.h>

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <source-file> <destination-file>\n", argv[0]);
		return -1;
	}
	int file1 = open(argv[1], O_RDONLY);
	if(file1 == -1)
	{
		fprintf(stderr, "ERROR: Unable to open file '%s', errno = %d (%s) \n", argv[1], errno, strerror(errno));
		return -1;
	}
	int file2 = open(argv[2], O_WRONLY | O_APPEND | O_CREAT);
	if(file2 == -1)
	{
		fprintf(stderr, "ERROR: Unable to open file '%s', errno = %d (%s) \n", argv[2], errno, strerror(errno));
		return -1;
	}
	char buffer[8];
	int character;
	while((character = read(file1, buffer, 8)) > 0)
	{
		if(write(file2, buffer, character) != character)
		{
			fprintf(stderr, "ERROR: Problem while writing", errno, strerror(errno));
			return -1;
		}
	}
	printf("Concatenated file contents to %s \n", argv[2]);
	close(file1);
	close(file2);
	return 0;
}
