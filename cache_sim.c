#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
//some definitions
#define FALSE 0
#define TRUE 1
#define ADDR long long
#define BOOL char

typedef struct _MEMACCESS
{
	ADDR addr;
	BOOL is_read;
} MEMACCESS;

typedef struct _Cache
{
	int valid;
	long long tag;
	long long data;
	int counter;
} Cache;

typedef enum _RPL
{
	LRU = 0,
	RAND = 1
} RPL;

//misc. function
FILE *fp = 0;
char trace_file[100] = "memtrace.trc";
BOOL read_new_memaccess(MEMACCESS *); //read new memory access from the memory trace file (already implemented)

//configure the cache
void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy);

//check if the memory access hits on the cache
BOOL isHit(ADDR addr, RPL repl_policy, int assoc);

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR addr, RPL repl_policy, int assoc);

//print the simulation statistics
void print_stat(int cache_size, int block_size, int assoc, RPL rep_policy);

Cache **cache;
int tag_bits;
int index_bits;
int byte_offset_bits;
int hit_count;
int miss_count;
long long add_tag;
long long add_index;
long long add_offset;
long long tag_mask;
long long index_mask;
long long offset_mask;

//main
int main(int argc, char *argv[])
{
	int cache_size = 1024;
	int assoc = 16;
	int block_size = 32;
	RPL repl_policy = RAND;
	int i;
	/*
	*  Read through command-line arguments for options.
	*/
	//srand((unsigned)time(NULL));
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (argv[i][1] == 's')
				cache_size = atoi(argv[i + 1]);

			if (argv[i][1] == 'b')
				block_size = atoi(argv[i + 1]);

			if (argv[i][1] == 'a')
				assoc = atoi(argv[i + 1]);

			if (argv[i][1] == 'f')
				strcpy(trace_file, argv[i + 1]);

			if (argv[i][1] == 'r')
			{
				if (strcmp(argv[i + 1], "lru") == 0)
					repl_policy = LRU;
				else if (strcmp(argv[i + 1], "rand") == 0)
				{
					repl_policy = RAND;
					srand((unsigned)time(NULL));
				}
				else
				{
					printf("unsupported replacement policy:%s\n", argv[i + 1]);
					return -1;
				}
			}
		}
	}

	/*
	 * main body of cache simulator
	*/

	init_cache(cache_size, block_size, assoc, repl_policy);

	//rand_assoc_count = assoc;
	while (1)
	{
		MEMACCESS new_access;

		BOOL success = read_new_memaccess(&new_access); //read new memory access from the memory trace file

		if (success != TRUE) //check the end of the trace file
			break;

		tag_mask = (long long)pow(2, tag_bits) - 1;
		index_mask = (long long)pow(2, index_bits) - 1;
		offset_mask = (long long)pow(2, byte_offset_bits) - 1;

		add_tag = (new_access.addr >> (index_bits + byte_offset_bits)) & tag_mask;
		add_index = (new_access.addr >> (byte_offset_bits)) & index_mask;

		add_offset = new_access.addr & offset_mask;

		if (isHit(new_access.addr, repl_policy, assoc) == FALSE) //check if the new memory access hit on the cache
		{
			insert_to_cache(new_access.addr, repl_policy, assoc); //if miss, insert a cache block for the memory access to the cache
		}
	}

	// print statistics here
	print_stat(cache_size, block_size, assoc, repl_policy);

	for (i = 0; i < assoc; i++)
		free(cache[i]);
	free(cache);
	return 0;
}

/*
 * read a new memory access from the memory trace file
 */
BOOL read_new_memaccess(MEMACCESS *mem_access)
{
	ADDR access_addr;
	char access_type[10];
	/*
	 * open the mem trace file
	 */

	if (fp == NULL)
	{
		fp = fopen(trace_file, "r");
		if (fp == NULL)
		{
			fprintf(stderr, "error opening file");
			exit(2);
		}
	}

	if (mem_access == NULL)
	{
		fprintf(stderr, "MEMACCESS pointer is null!");
		exit(2);
	}

	if (fscanf(fp, "%llx %s", &access_addr, &access_type) != EOF)
	{
		mem_access->addr = access_addr;
		if (strcmp(access_type, "RD") == 0)
			mem_access->is_read = TRUE;
		else
			mem_access->is_read = FALSE;

		return TRUE;
	}
	else
		return FALSE;
}

void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy)
{
	int set_index;
	set_index = (cache_size / block_size) / assoc;

	cache = (Cache **)malloc(sizeof(*cache) * set_index);
	int i = 0, j = 0, k = 0;

	for (i = 0; i < set_index; i++)
		cache[i] = (Cache *)malloc(sizeof(**cache) * assoc);

	if (repl_policy == 0)
	{
		for (i = 0; i < set_index; i++)
		{
			k = 0;
			for (j = 0; j < assoc; j++)
			{
				(cache[i][j]).counter = k++; //LRU counter 0부터 assoc-1까지 각각 초기화

				cache[i][j].valid = 0;
				//cache[i][j].tag = 0;//valid는 전부 0으로 초기화
			}
		}
	}
	else
	{
		for (i = 0; i < set_index; i++)
		{

			for (j = 0; j < assoc; j++)
			{

				cache[i][j].valid = 0; //valid는 0으로
			}
		}
	}
	index_bits = (int)log2((double)set_index);
	byte_offset_bits = (int)log2((double)block_size);
	tag_bits = 64 - (index_bits + byte_offset_bits);
}

//check if the memory access hits on the cache
BOOL isHit(ADDR addr, RPL repl_policy, int assoc)
{
	int i, j;
	if (repl_policy == 0)
	{
		for (j = 0; j < assoc; j++)
		{
			if (add_tag == cache[add_index][j].tag && (cache[add_index][j].valid) != 0)
			{

				for (i = 0; i < assoc; i++)
				{
					if (cache[add_index][j].counter < cache[add_index][i].counter)
					{
						(cache[add_index][i].counter)--;
					}
				}

				(cache[add_index][j].counter) = assoc - 1;
				cache[add_index][j].valid = 1;
				hit_count++;
				break;
			}

			//else
			//miss_count++;
			//마지막 assoc다 돌았을때
			else if ((j == assoc - 1) && (add_tag != cache[add_index][j].tag) || (cache[add_index][j].valid) == 0)
			{

				miss_count++;
				//insert_to_cache(addr,add_tag,add_index,add_offset);
				return FALSE;
			}
		}
	}

	//RAND
	else
	{
		for (j = 0; j < assoc; j++)
		{
			if (add_tag == cache[add_index][j].tag && (cache[add_index][j].valid) != 0)
			{

				cache[add_index][j].valid = 1;
				hit_count++;
				break;
			}

			//else
			//miss_count++;
			else if ((j == assoc - 1) && (add_tag != cache[add_index][j].tag) || (cache[add_index][j].valid) == 0)
			{

				miss_count++;
				//insert_to_cache(addr,add_tag,add_index,add_offset);
				return FALSE;
			}
		}
	}

	return TRUE;
}

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR addr, RPL repl_policy, int assoc)
{
	//int initial_rand = FALSE;
	int random;
	int i;

	if (repl_policy == 0)
	{
		for (i = 0; i < assoc; i++)
		{
			if (cache[add_index][i].counter == 0)
			{
				cache[add_index][i].valid = 1;
				cache[add_index][i].tag = add_tag;
				cache[add_index][i].counter = assoc - 1;
			}
			else
				(cache[add_index][i].counter)--;
		}
	}

	else
	{

		random = rand() % assoc;
		cache[add_index][random].tag = add_tag;
		cache[add_index][random].valid = 1;
	}
	return TRUE;
}

//print the simulation statistics
void print_stat(int cache_size, int block_size, int assoc, RPL repl_policy)
{

	printf("cache_size : %d B\n", cache_size);
	printf("block_size : %d B\n", block_size);
	printf("associativity : %d\n", assoc);
	if (repl_policy == 0)
		printf("replacement policy : %s\n", "LRU");
	else
		printf("replacement policy : %s\n", "RAND");

	printf("cache accesses : %d\n", hit_count + miss_count);
	printf("cache_hits : %d\n", hit_count);
	printf("cache_misses : %d\n", miss_count);
	printf("cache_miss_rate : %.2lf%\n", 100 * ((double)miss_count / (hit_count + miss_count)));
}
