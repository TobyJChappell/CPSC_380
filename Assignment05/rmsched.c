/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 5

Compile: gcc rmsched.c -o rmsched -pthread

Run: ./rmsched <nperiods> <task set> <schedule>

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

int numTasks;
int nperiods;
sem_t control;
sem_t *sems;

struct Task
{
	char name[10];
	int wcet;
	int period;
	int remaining_time;
	int id;
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
		sems = (sem_t *)malloc(numTasks*sizeof(sem_t));

		//retrieve contents of task set and store in tasks
		task_set = fopen(argv[2],"r");
		int i;
    for(i = 0; i < numTasks; i++)
		{
			if(fscanf(task_set, "%s %d %d", tasks[i].name, &tasks[i].wcet, &tasks[i].period) != 3)
			{
				printf("Task set format error on line %d\n",i+1);
        return -1;
			}
			tasks[i].remaining_time = 0;
			tasks[i].id = i;
		}

		fclose(task_set);

		//calculate hyperperiod and multiple to nperiods
		int h_period = tasks[0].period;
    for (i = 1; i < numTasks; i++)
		{
			h_period = (((tasks[i].period*h_period))/(gcd(tasks[i].period, h_period)));
		}

		//check if task set is able to be scheduled
		int priority, index, j;
		for(i = 0; i < h_period; i++)
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
						printf("Task set not possible to schedule\n");
						return 0;
					}
				}
			}

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

			//decrement remaining_time
			if(index != -1)
			{
				tasks[index].remaining_time--;
			}
		}

		FILE *schedule = fopen(argv[3], "w");
		//write clock ticks to scheduler
		for(i = 0; i < h_period; i++)
		{
			fprintf(schedule, "%d ", i);
		}
		fputc('\n', schedule);

		//intialize semaphores
		for(i = 0; i < numTasks; i++)
		{
			if(sem_init(&sems[i],0,0) != 0)
			{
				printf("Failed to initialize semaphore\n");
				return -1;
			}
		}

		if(sem_init(&control,0,0) != 0)
		{
			printf("Failed to initialize semaphore\n");
			return -1;
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
		int k;
		for(i = 0; i < nperiods; i++)
		{
			for(j = 0; j < h_period; j++)
			{
				//update remaining_time
				for(k = 0; k < numTasks; k++)
				{
					if(j%tasks[k].period == 0)
					{
						if(tasks[k].remaining_time == 0)
						{
							tasks[k].remaining_time = tasks[k].wcet;
						}
					}
				}

				//determine which task gets to run
				priority = -1;
				index = -1;
				for(k = 0; k < numTasks; k++)
				{
					if(tasks[k].remaining_time != 0 && 1/tasks[k].period > priority)
					{
						priority = tasks[k].period;
						index = k;
					}
				}

				//release task semaphore
				if(index == -1)
				{
					printf("Idle\n");
					fputs("I ", schedule);
				}
				else
				{
					//allow task to run
					if(sem_post(&sems[index]) != 0)
					{
							printf("Failed to unlock semaphore\n");
							return -1;
					}
					if(sem_wait(&control) != 0)
					{
						printf("Failed to lock semaphore\n");
						return -1;
					}
					tasks[index].remaining_time--;
					fprintf(schedule, "%s ", tasks[index].name);
				}
			}
			fputc('\n', schedule);
		}
		printf("Scheduling successful!\n");

		fclose(schedule);

		//destroy semaphores
		for(i = 0; i < numTasks; i++)
		{
			if(sem_destroy(&sems[i]) != 0)
			{
		    printf("Failed to destroy semaphore\n");
				return -1;
			}
		}
		if(sem_destroy(&control) != 0)
		{
			printf("Failed to destroy semaphore\n");
			return -1;
		}

		free(sems);
    return 0;
}

/**
Prints name of task running
*/
void *task(void *arg)
{
	int index = ((struct Task*)arg)->id;
	char *name = ((struct Task*)arg)->name;
	while(1)
	{
		if(sem_wait(&sems[index]) != 0)
		{
			printf("Failed to lock semaphore\n");
			exit(EXIT_FAILURE);
		}
		printf("Task Name: %s is running\n", name);
		fflush(stdout);
		if(sem_post(&control) != 0)
		{
			printf("Failed to unlock semaphore\n");
			exit(EXIT_FAILURE);
		}
	}
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
