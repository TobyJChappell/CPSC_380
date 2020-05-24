/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 7

The program implements the following disk-scheduling algorithms:
a.	FCFS
b.	SSTF
c.	SCAN
d.	C-SCAN
e.	LOOK
f.	C-LOOK

Compile: gcc diskScheduler.c -o diskScheduler

Run: ./diskScheduler <initial cylinder position> <cylinder request file>
<initial cylinder position> = initial position should be an integer
<cylinder request file> = list of addresses numbered 0-4999 seperaated by new lines
*/

#include <stdio.h>
#include <stdlib.h>

int init;
int init_pos;
int length;
int *addr;

int fcfs();
int sstf();
int scan();
int c_scan();
int look();
int c_look();

void sort();
int pos();

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <initial cylinder position> <cylinder request file>\n", argv[0]);
		return -1;
	}

	int temp = atoi(argv[1]);
	if(temp < 0 || temp > 4999)
	{
		printf("Initial cylinder position must be in the range 0-4999\n");
		return -1;
	}
	init = temp;

	FILE *address_file = fopen(argv[2], "r");
	if(address_file == NULL)
  {
		printf("Cylinder request file does not exist.");
		return -1;
	}

	length = 0;
	int test;
	while(fscanf(address_file, "%d", &test) == 1)
	{
		length++;
	}
	fclose(address_file);

	if(length == 0)
	{
		printf("File is not formatted correctly\n");
		return -1;
	}
	addr = malloc(length*sizeof(int));
	address_file = fopen(argv[2], "r");
	int i = 0;
	while(fscanf(address_file, "%d", &addr[i]) == 1)
	{
		i++;
	}
	fclose(address_file);
	printf("Total Head Movement for FCFS: %d\n", fcfs());
	sort();
	init_pos = pos();
	printf("Total Head Movement for SSTF: %d\n", sstf());
	printf("Total Head Movement for SCAN: %d\n", scan());
	printf("Total Head Movement for C-SCAN: %d\n", c_scan());
	printf("Total Head Movement for LOOK: %d\n", look());
	printf("Total Head Movement for C-LOOK: %d\n", c_look());
  return 0;
}

/**
Performs FCFS on addresses
@return int Total movement
*/
int fcfs()
{
	int movt = abs(init-addr[0]);
	int i;
	for(i = 1; i < length; i++)
	{
		movt += abs(addr[i]-addr[i-1]);
	}
	return movt;
}

/**
Performs SSTF on addresses
@return int Total movement
*/
int sstf()
{
	int i;
	int pos = init_pos;
	int movt = abs(init - addr[init_pos]);
	int lower_pos = pos - 1;
	int upper_pos = pos + 1;
	int lower_movt = 0;
	int upper_movt = 0;
	for(i = 0; i < length-1; i++)
	{
		lower_movt = abs(addr[pos]-addr[lower_pos]);
		upper_movt = abs(addr[pos]-addr[upper_pos]);
		if(lower_pos >= 0 && lower_movt < upper_movt)
		{
			movt += lower_movt;
			pos = lower_pos;
			lower_pos--;
		}
		else
		{
			movt += upper_movt;
			pos = upper_pos;
			upper_pos++;
		}
	}
	return movt;
}

/**
Performs SCAN on addresses
@return int Total movement
*/
int scan()
{
	int i;
	int movt = abs(init - addr[init_pos]);
	for(i = init_pos+1; i < length; i++)
	{
		movt += abs(addr[i]-addr[i-1]);
	}
	if(init_pos != 0)
	{
		movt += abs(addr[length-1]-4999);
		movt += abs(4999-addr[init_pos-1]);
		for(i = init_pos-1; i > 1; i--)
		{
			movt += abs(addr[i]-addr[i-1]);
		}
	}
	return movt;
}

/**
Performs C-SCAN on addresses
@return int Total movement
*/
int c_scan()
{
	int i;
	int movt = abs(init - addr[init_pos]);
	for(i = init_pos+1; i < length; i++)
	{
		movt += abs(addr[i]-addr[i-1]);
	}
	if(init_pos != 0)
	{
		movt += abs(addr[length-1]-4999);
		movt += abs(4999-0);
		movt += abs(0-addr[0]);
		for(i = 1; i < init_pos-1; i++)
		{
			movt += abs(addr[i]-addr[i-1]);
		}
	}
	return movt;
}

/**
Performs LOOK on addresses
@return int Total movement
*/
int look()
{
	int i;
	int movt = abs(init - addr[init_pos]);
	for(i = init_pos+1; i < length; i++)
	{
		movt += abs(addr[i]-addr[i-1]);
	}
	if(init_pos != 0)
	{
		movt += abs(addr[length-1]-addr[init_pos-1]);
		for(i = init_pos-1; i > 1; i--)
		{
			movt += abs(addr[i]-addr[i-1]);
		}
	}
	return movt;
}

/**
Performs C-LOOK on addresses
@return int Total movement
*/
int c_look()
{
	int i;
	int movt = abs(init - addr[init_pos]);
	for(i = init_pos+1; i < length; i++)
	{
		movt += abs(addr[i]-addr[i-1]);
	}
	if(init_pos != 0)
	{
		movt += abs(addr[length-1]-addr[0]);
		for(i = 1; i < init_pos-1; i++)
		{
			movt += abs(addr[i]-addr[i-1]);
		}
	}
	return movt;
}

/**
Sorts addresses in ascending order
*/
void sort()
{
    int i, j, min_idx;
		int temp;
    for (i = 0; i < length-1; i++)
    {
        min_idx = i;
        for (j = i+1; j < length; j++)
          if (addr[j] < addr[min_idx])
            min_idx = j;
        temp = addr[min_idx];
				addr[min_idx] = addr[i];
				addr[i] = temp;
    }
}

/**
Returns the intial position location closest to the inital cylinder position (to the right)
@return int Initial position in address list
*/
int pos()
{
	int i = 0;
	for(i = 0; i < length; i++)
	{
		if(addr[i] >= init)
		{
			return i;
		}
	}
	return length - 1;
}
