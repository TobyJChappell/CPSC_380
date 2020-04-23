/**
@name Toby Chappell
@student_id 2312642
@email tchappell@chapman.edu
@course CPSC-380
@assignment 6



Compile: gcc vmmgr.c -o vmmgr

Run: ./vmmgr <addresses> <backing store>

References:
https://stackoverflow.com/questions/8011700/how-do-i-extract-specific-n-bits-of-a-32-bit-unsigned-integer-in-c
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

unsigned createMask(unsigned i, unsigned f);

int main(int argc, char *argv[])
{
		if(argc != 3)
		{
			fprintf(stderr, "Usage: %s <addresses> <backing store>\n", argv[0]);
			return -1;
		}

		FILE *addresses = fopen(argv[2], "r");
		if(addresses == NULL)
    {
        printf("Address file does not exist.");
        return -1;
    }

		FILE *backing_store = fopen(argv[2], "r");
		if(backing_store == NULL)
    {
        printf("Backing store does not exist.");
        return -1;
    }
		fclose(backing_store);

		int i, j;
		int tlb[16][2];
		for(i = 0; i < 16; i++)
		{
			for(j = 0; j < 2; j++)
			{
				tlb[i][j] = -1;
      }
		}

		int page_table[256];
		char physical_memory[256*256];
		for(i = 0; i < 256; i++)
		{
			page_table[i] = -1;
		}


		unsigned offset_mask = createMask(0,7);
		unsigned page_num_mask = createMask(8,15);
		int v_address, p_address, offset, page_num, frame_num;
		int index = 0;
		int num_faults = 0;
		int num_hits = 0;
		int num_refs = 0;
		int page_hit = 0;
		int page_fault = 0;
		int fifo = 0;
		int sign;
		while(fscanf(addresses, "%d", &v_address) != 1)
		{
			page_num = page_num_mask & v_address;
			offset = offset_mask & v_address;
			i = 0;
			page_hit = 0;
			while(page_hit == 0 && i < 16)
			{
				if(tlb[i][0] == page_num)
				{
					page_hit = 1;
					num_hits++;
					frame_num = tlb[i][1];
				}
			}
			if(page_hit == 0)
			{
				if(page_table[page_num] == -1)
				{
					fseek(backing_store, page_num*256, SEEK_SET);
					fread(physical_memory, 1, 256, backing_store);
					tlb[fifo][0] = page_num;
					tlb[fifo][1] = frame_num;
					fifo++;
					if(fifo == 16)
					{
						fifo = 0;
					}
					num_faults++;
				}
			}

			printf("Virtual Address: %d, Physical Address: %d Signed Byte Value: %d\n", v_address, p_address, sign);
			num_refs++;
		}
		double fault_rate = (double)num_faults/num_refs;
		double hit_rate = (double)num_hits/num_refs;

		printf("Page Faults: %d", num_faults);
		printf("Page Fault Rate: %lf", fault_rate);
		printf("TLB Hits: %d", num_hits);
		printf("TLB Hit Rate: %lf", hit_rate);
		fclose(addresses);
		fclose(backing_store);
    return 0;
}

//mmap for backing store EC

/**
Creates a mask starting at init and ending at fin
@param i Starting position of mask (should be <= to f)
@param f Last position of mask
@return mask
*/
unsigned createMask(unsigned i, unsigned f)
{
	if(i > f)
	{
		return -1;
	}
	unsigned mask = 0;
	unsigned x;
	for(x = i; i <= f; x++)
	{
		mask |= 1 << x;
	}
	return mask;
}
