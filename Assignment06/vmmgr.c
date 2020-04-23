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

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <addresses> <backing store>\n", argv[0]);
		return -1;
	}
	FILE *addresses = fopen(argv[1], "r");
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
	for(i = 0; i < 256; i++)
	{
		page_table[i] = -1;
	}

	unsigned char p_mem[256][256];
	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < 256; j++)
		{
			p_mem[i][j] = 0;
		}
	}

	int v_address, p_address;
	int offset, page_num, frame_num;
	int num_faults, num_hits, num_refs = 0;
	int p_mem_frame = 0;
	int page_hit = 0;
	int fifo = 0;
	unsigned char s_byte;
	while(fscanf(addresses, "%d", &v_address) == 1)
	{
		v_address = ((1 << 16) - 1) & v_address;
		page_num = v_address >> 8;
		offset = ((1 << 8) - 1) & v_address;
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
			i++;
		}
		if(page_hit == 0)
		{
			if(page_table[page_num] == -1)
			{
				fseek(backing_store, page_num*256, SEEK_SET);
				fread(p_mem[p_mem_frame], 1, 256, backing_store);
				tlb[fifo][0] = page_num;
				tlb[fifo][1] = p_mem_frame;
				page_table[page_num] = p_mem_frame;
				fifo++;
				if(fifo == 16)
				{
					fifo = 0;
				}
				num_faults++;
				p_mem_frame++;
			}
			frame_num = page_table[page_num];
		}
		p_address = frame_num << 8 | offset;
		s_byte = p_mem[frame_num][offset];
		printf("Virtual Address: %d, Physical Address: %d, Signed Byte Value: %u\n", v_address, p_address, s_byte);
		num_refs++;
	}
	double fault_rate = (double)num_faults/num_refs;
	double hit_rate = (double)num_hits/num_refs;

	printf("Page Faults: %d\n", num_faults);
	printf("Page Fault Rate: %lf\n", fault_rate);
	printf("TLB Hits: %d\n", num_hits);
	printf("TLB Hit Rate: %lf\n", hit_rate);
	fclose(addresses);
	fclose(backing_store);
  return 0;
}
