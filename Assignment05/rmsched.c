/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 5



Compile: gcc rmsched.c -o rmsched -pthread

Run: ./rmsched

References:
http://www.cse.cuhk.edu.hk/~ericlo/teaching/os/lab/9-PThread/Pass.html
https://cboard.cprogramming.com/c-programming/38639-reading-columns-input-data-file.html
https://www.geeksforgeeks.org/structures-c/
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

void *task(void *arg);
int gcd(int a, int b);

void *ptr;
int numTasks;
int nperiods;

struct Task
{
	char name[10];
	int wcet;
	int period;
	int remaining_time;
	sem_t sem;
	pthread_t thread;
};

int main(int argc, char *argv[])
{
		if(argc != 4)
		{
			fprintf(stderr, "Usage: %s <nperiods> <task set> <schedule>\n", argv[0]);
			return -1;
		}

		int temp = atoi(argv[1]);
		if(temp < 1)
		{
			printf("Number of hyperperiods must be a valid integer greater than 0\n");
			return -1;
		}
		nperiods = temp;

		//check if task set exists
		FILE *task_set = fopen(argv[2], "r");
		if (task_set == NULL)
    {
        printf("Task set does not exist.");
        return -1;
    }

		//get number of lines of task set
    int num_lines = 0;
    char c;
    while ((c=fgetc(task_set))!=EOF)
    {

        if (c == '\n')
        {
            num_lines++;
        }
    }
    fclose(task_set);

		numTasks = num_lines;
		struct Task tasks[numTasks];

		//retrieve contents of task set and store in tasks
		task_set = fopen(argv[2],"r");
		int i;
    for(i = 0; i < numTasks; i++)
		{
			if(fscanf(task_set, "%s %d %d", tasks[i].name, &tasks[i].wcet, &tasks[i].period) != 3 )
			{
				printf("Task set format error at line %d.",i);
        return -1;
			}
			tasks[i].remaining_time = 0;
		}
		fclose(task_set);

		//print tasks
		for(i = 0; i < numTasks; i++)
		{
			printf("Task Name: %s, WCET: %d, Period: %d\n",tasks[i].name, tasks[i].wcet,tasks[i].period);
		}

		//calculate hyperperiod and multiple to nperiods
		int h_period = tasks[0].period;
    for (i = 1; i < numTasks; i++)
		{
			h_period = (((tasks[i].period*h_period))/(gcd(tasks[i].period, h_period)));
		}
		nperiods *= h_period;

		void *thread_result;

		//intialize semaphores
		for(i = 0; i < numTasks; i++)
		{
			if((sem_init(&tasks[i].sem,0,i) != 0))
			{
				printf("Failed to initialize semaphore\n");
				return -1;
			}
		}

		//create threads
		for(i = 0; i < numTasks; i++)
		{
			if(pthread_create(&tasks[i].thread, NULL, task, (void*)&tasks[i]) != 0)
			{
	        printf("Thread creation failed\n");
	        return -1;
	    }
		}

		//schedule tasks
		int priority, index, j;
		for(i = 0; i < nperiods; i++)
		{
			//update remaining_time and check if any missed deadlines
			for(j = 0; j < numTasks; j++)
			{
				if(i%tasks[j].period == 0)
				{
					if(tasks[j].remaining_time == 0)
					{
						tasks[j].remaining_time = tasks[j].wcet;
					}
					else
					{
						printf("Missed deadline for task %s on clock tick %d\n", tasks[j].name,i);
						return 0;
					}
				}
			}

			//determine which task gets to run
			priority = -1;
			index = -1;
			for(j = 0; j < numTasks; j++)
			{
				if(tasks[j].remaining_time != 0 && 1/tasks[j].period > priority)
				{
					priority = tasks[j].period;
					index = j;
				}
			}

			//release task semaphore
			if(index == -1)
			{
				printf("Idle\n");
			}
			else
			{
				//allow task to run
				if(sem_post(&tasks[index].sem) != 0)
				{
						printf("Failed to unlock semaphore\n");
						return -1;
				}
				printf("Task Name: %s is running\n", tasks[index].name);
				tasks[index].remaining_time = tasks[index].remaining_time-1;
			}
		}

		printf("Scheduling successful!\n");

		//join threads
		for(i = 0; i < numTasks; i++)
		{
			if(pthread_join(tasks[i].thread, &thread_result) != 0)
			{
	        printf("Thread join failed\n");
	        return -1;
	    }
		}

		//destroy semaphores
		for(i = 0; i < numTasks; i++)
		{
			if((sem_destroy(&tasks[i].sem) != 0))
			{
		    printf("Failed to destroy semaphore\n");
				return -1;
			}
		}
    return 0;
}

/**
Prints name of task running
*/
void *task(void *arg)
{
	/**
	while(1)
	{
		if(sem_wait(&((struct Task*)arg)->sem) != 0)
		{
			printf("Failed to lock semaphore\n");
			exit(EXIT_FAILURE);
		}
		printf("Task Name: %s is running\n", ((struct Task*)arg)->name);
		((struct Task*)arg)->remaining_time--;
		fflush(stdout);
	}
	*/
	pthread_exit(0);
}

/**
Returns the gcd of two numbers
*/
int gcd(int i, int j)
{
    if (j == 0)
		{
			return i;
		}
    return gcd(j, i%j);
}
