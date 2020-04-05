/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 4

Creates a producer and consumer thread synchonized through the use of mutex semaphores. The producer writes 30 bytes of random data followed by the Internet checksum into a shared memory block. The consumer reads the data from the shared memory block and checks the checksum to make sure data has not been corrupted. This is to be conducted ntimes as defined by the user.

Compile: gcc prodcon.c -o prodcon -pthread

Run: ./prodcon <memsize> <ntimes>
Where memsize is the size of the shared memory and ntimes is the number of times the producer produces and consumer consumes.

References:
https://stackoverflow.com/questions/4641476/using-dynamic-memory-allocation-for-arrays
http://man7.org/linux/man-pages/man3/sem_init.3.html
http://man7.org/linux/man-pages/man3/sem_post.3.html
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

void *consumer(void *arg);
void *producer(void *arg);
unsigned int ip_checksum(char *data, int length);

sem_t sem[2];
pthread_mutex_t work_mutex;
void *ptr;
unsigned char *work_area;
int memsize;
int nblocks;
int ntimes;

int main(int argc, char *argv[]) {
		if(argc != 3)
		{
			fprintf(stderr, "Usage: %s <memsize> <ntimes>\n", argv[0]);
			return -1;
		}

		int temp = atoi(argv[1]);
		if(temp < 32 || temp >= 64000)
		{
			printf("Memory size must be a valid integer greater than 31 and less than 64K\n");
			return -1;
		}
		nblocks = temp/32; //number of blocks of shared memory
		memsize = nblocks*32; //size of shared memory defined by user (changed to a multiple of 32)
		work_area = malloc(memsize); //allocate memory

		temp = atoi(argv[2]);
		if(temp < 1)
		{
			printf("Number of times must be a valid integer greater than 0\n");
			return -1;
		}
		ntimes = temp; //number of times to execute defined by user

		//initialize semaphore
		if((sem_init(&sem[0],0,1) != 0) || (sem_init(&sem[1],0,0) != 0))
		{
			printf("Failed to initialize semaphore\n");
			return -1;
		}

		//initialize mutex
    if(pthread_mutex_init(&work_mutex, NULL) != 0)
		{
        return -1;
    }

		pthread_t p_thread;
		pthread_t c_thread;
		void *thread_result;

		//create producer and consumer thread
    if((pthread_create(&p_thread, NULL, producer, NULL) != 0) || (pthread_create(&c_thread, NULL, consumer, NULL) != 0))
		{
        printf("Thread creation failed\n");
        return -1;
    }

		//join producer and consumer thread
		if((pthread_join(p_thread, &thread_result) != 0) || (pthread_join(c_thread, &thread_result) != 0))
		{
        printf("Thread join failed\n");
        return -1;
    }

		//destroy semaphore
		if((sem_destroy(&sem[0]) != 0) || (sem_destroy(&sem[1]) != 0))
		{
	    printf("Failed to destroy semaphore\n");
			return -1;
		}

		free(work_area);
    pthread_mutex_destroy(&work_mutex);
    return 0;
}

/**
Creates 30 bytes of random data (0-255) and then stores the checksum in the last 2 bytes of the shared memory block. This is done ntimes synchronized with the consumer
*/
void *producer(void *arg)
{
	int i, j, k, l;
	char p_data[30];
	for(i = 0; i < ntimes; i++)
	{
		//waits until consumer is done reading
		if(sem_wait(&sem[0]) != 0)
		{
			printf("Failed to lock semaphore\n");
			exit(EXIT_FAILURE);
		}
		for(j = 0; j < nblocks; j++)
		{
			//lock mutex
			pthread_mutex_lock(&work_mutex);

			//write data 30 bytes of random data
			l = 0;
			for(k = j*32; k < j*32+30; k++)
			{
				work_area[k] = rand()%256;
				p_data[l] = work_area[k];
				l++;
			}

			//calculate and store checksum
			((unsigned short *)work_area)[(k+1)/2] = ip_checksum(p_data,strlen(p_data));

			//unlock mutex
			pthread_mutex_unlock(&work_mutex);

			//allow consumer to read
			if(sem_post(&sem[1]) != 0)
			{
					printf("Failed to unlock semaphore\n");
					exit(EXIT_FAILURE);
			}
		}
	}
	pthread_exit(0);
}

/**
Reads from the shared memory buffer of 30 bytes, calculates the checksum based upon the 30 bytes and compares that with the value stored in the shared memory. This is done ntimes synchronized with the producer.
*/
void *consumer(void *arg)
{
		int i, j, k, l;
		unsigned short c_cksum;
		unsigned short p_cksum;
		char c_data[30];
		for(i = 0; i < ntimes; i++)
		{
			for(j = 0; j < nblocks; j++)
			{
				//waits if there is no data to read
				if(sem_wait(&sem[1]) != 0)
				{
					printf("Failed to lock semaphore\n");
					exit(EXIT_FAILURE);
				}

				//lock mutex
        pthread_mutex_lock(&work_mutex);

				//calculate checksum from data
				l = 0;
				for(k = j*32; k < j*32+30; k++)
				{
					c_data[l] = work_area[k];
					l++;
        }
				c_cksum = ip_checksum(c_data,strlen(c_data));

				//read in checksum
				p_cksum = ((unsigned short *)work_area)[(k+1)/2];

				//check checksum
				if(p_cksum != c_cksum)
				{
					printf("Checksums read and produced did not match\n");
					printf("Data Found: %s\n", c_data);
					printf("Expected Checksum: 0x%x\n", p_cksum);
					printf("Calculated Checksum: 0x%x\n", c_cksum);
					exit(EXIT_FAILURE);
        }
				//unlock mutex
				pthread_mutex_unlock(&work_mutex);
			}

			//allows producer to write
			if(sem_post(&sem[0]) != 0)
			{
				printf("Failed to unlock semaphore\n");
				exit(EXIT_FAILURE);
	    }
		}
    pthread_exit(0);
}

/**
Calculates the Internet Checksum from a piece of data of some length
@param data 30 bytes of random data
@param length Length of data
@return unsigned short int Internet Checksum
*/
unsigned int ip_checksum(char *data, int length)
{
    unsigned int sum = 0xffff;
    unsigned short word;
    int  i;

    // Handle complete 16-bit blocks.
    for (i = 0; i+1<length; i+=2) {
        memcpy(&word, data+i, 2);
        sum += word;
        if (sum > 0xffff) {
            sum -= 0xffff;
        }
    }
    // Handle any partial block at the end of the data.
    if (length&1) {
        word=0;
        memcpy(&word, data+length-1, 1);
        sum += word;;
        if (sum > 0xffff) {
            sum -= 0xffff;
        }
    }
    // Return the checksum
    return ~sum;

}
