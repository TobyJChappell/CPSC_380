/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 3

Estimates the value of pi using pthreads. This is calculated using the equation: pi = 4 * hit_count/total_pts, where hit_count is the number of points inside the circle and total_pts is the total number of points.

Compile: gcc mcarlo.c -o mcarlo -plthread -ml

References: https://stackoverflow.com/questions/9748393/how-can-i-get-argv-as-int/38669018
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

int hit_count;
int total_pts;

double random_double();
void *point_in_circle();

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <number of points>\n", argv[0]);
		return -1;
	}

	char *p;
	errno = 0;
	long temp = strtol(argv[1], &p, 10);
	if (errno != 0 || *p != '\0' || temp > INT_MAX || temp < 1)
	{
		printf("Number of points must be a valid integer greater than 0\n");
		return -1;
	}
	total_pts = temp;

	pthread_t tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_create(&tid,&attr,point_in_circle,NULL);
	pthread_join(tid,NULL);
	double pi = 4 * ((double)hit_count)/((double)total_pts);
	printf("pi = %f\n",pi);

	return 0;
}

/*
Counts the number of points inside the circle.
@param *param
*/
void *point_in_circle(void *param)
{
	double x = 0;
	double y = 0;
	int i;
	for(i = 0; i < total_pts; i++)
	{
		x = random_double() * 2.0 - 1.0;
		y = random_double() * 2.0 - 1.0;
		if (sqrt(x*x + y*y) < 1.0 )
		{
			++hit_count;
		}
	}
	pthread_exit(0);
}

/*
Generates a double precision random number
@return double
*/
double random_double()
{
	return random() / ((double)RAND_MAX + 1);
}
