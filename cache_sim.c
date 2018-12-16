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

typedef struct _MEMACCESS {
	ADDR addr;
	BOOL is_read;
} MEMACCESS;

typedef struct _Cache {
	int valid;
	long long tag;
	int data;
	int counter;
}Cache;

Cache **cache;
int tag_bits;
int index_bits;
int byte_offset_bits;

typedef enum _RPL { LRU = 0, RAND = 1 } RPL;

//misc. function
FILE* fp = 0;
char trace_file[100] = "memtrace.trc";
BOOL read_new_memaccess(MEMACCESS*);  //read new memory access from the memory trace file (already implemented)


//configure the cache
void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy);

//check if the memory access hits on the cache
BOOL isHit(ADDR addr,RPL repl_policy);

//insert a cache block for a memory access
ADDR insert_to_cache(ADDR addr, RPL repl_policy);


//print the simulation statistics
print_stat(RPL rep_policy);

int a;
int b;
int access_count;
int hit_count;
int miss_count;
int i = 0;
int cache_size = 1024;
int assoc = 4;
int block_size = 32;
RPL repl_policy = RAND;
int rand_assoc_count = 0;
long long add_tag;
long long add_index;
long long add_offset;
long long tag_mask;
long long index_mask;
long long offset_mask;
//main
int main(int argc, char*argv[])
{
	
	
	/*
	*  Read through command-line arguments for options.
	*/
	
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
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
					repl_policy = RAND;
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

	init_cache(cache_size, block_size, assoc,  repl_policy);


	rand_assoc_count = assoc;
	while (1)
	{
		MEMACCESS new_access;

		BOOL success = read_new_memaccess(&new_access);  //read new memory access from the memory trace file

		if (success != TRUE)   //check the end of the trace file
			break;


		tag_mask = pow(2.0, tag_bits) - 1;
		index_mask = pow(2.0, index_bits) - 1;
		offset_mask = pow(2.0, byte_offset_bits) - 1;

		add_tag = (new_access.addr >> (index_bits + byte_offset_bits)) & tag_mask;
		add_index = (new_access.addr >> (byte_offset_bits))&index_mask;

		 add_offset = new_access.addr & offset_mask;

		
			if(isHit(new_access.addr,repl_policy)==FALSE)   //check if the new memory access hit on the cache
			{
				insert_to_cache(new_access.addr,  repl_policy);  //if miss, insert a cache block for the memory access to the cache
			}
		
			access_count = hit_count + miss_count;
			a++;
	}

	// print statistics here
	print_stat(repl_policy);
	//	free(cache);
	return 0;
}


/*
 * read a new memory access from the memory trace file
 */
BOOL read_new_memaccess(MEMACCESS* mem_access)
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

		b++;
		return TRUE;
	}
	else
		return FALSE;

}


void init_cache(int cache_size, int block_size, int assoc, RPL repl_policy)
{
	int set_index;
	


	set_index = (cache_size / block_size) / assoc;


	cache= (struct Cache **)malloc(sizeof(*cache)*set_index);
	int i=0;
	int j=0, k = 0;

	for (i = 0; i < set_index; i++)
		cache[i] = (struct cache *)malloc(sizeof(**cache)*assoc);
	

	i = 0;


	if (repl_policy == 0) {
		for (i = 0; i < set_index; i++) {
			k = 0;
			for (j = 0; j < assoc; j++) {
				(cache[i][j]).counter = k++;

				cache[i][j].valid = 0;
			}

		}
	}
	else
	{
		for (i = 0; i < set_index; i++) {
			
			for (j = 0; j < assoc; j++) {

				cache[i][j].valid = 0;
			}

		}

	}
	index_bits =log2(set_index);
	byte_offset_bits = log2(block_size);
	tag_bits = 64 - (index_bits + byte_offset_bits);

	// if((cache[i][j]).vaild==0 &&(cache[i][j]).counter==0


	
}

//check if the memory access hits on the cache
BOOL isHit(ADDR addr,RPL repl_policy)
{
	
	int i, j;
	if (repl_policy == 0) {
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
			else if ((j == assoc - 1) && (add_tag != cache[add_index][j].tag) || (cache[add_index][j].valid) == 0)	
			{

				miss_count++;
				//insert_to_cache(addr,add_tag,add_index,add_offset);
				return FALSE;
			}



		}
	}
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
ADDR insert_to_cache(ADDR addr, RPL repl_policy)
{

	
	//int initial_rand = FALSE;
	srand(time(NULL));
	int random;
	int i, j=0;
	/*long long tag_mask = pow(2.0, tag_bits) - 1;
	long long index_mask = pow(2.0, index_bits) - 1;
	long long offset_mask = pow(2.0, byte_offset_bits) - 1;

	long long add_tag = (addr >> (index_bits + byte_offset_bits)) & tag_mask;
	long long add_index = (addr >> (byte_offset_bits))&index_mask;

	long long add_offset = addr & offset_mask;*/
	if (repl_policy == 0) {
		for (i = 0; i < assoc; i++)
		{
			if (cache[add_index][i].counter == 0 )
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

		if (rand_assoc_count > 0)
		{
			while (cache[add_index][random].valid != 0)
				random = rand() % assoc;

			cache[add_index][random].tag = add_tag;
			cache[add_index][random].valid = 1;

			rand_assoc_count--;
		}

		else
		{
			cache[add_index][random].tag = add_tag;
			cache[add_index][random].valid = 1;
		}
		

	}
	return TRUE;
}


//print the simulation statistics
print_stat(RPL repl_policy)
{

	printf("cache_size : %d\n", cache_size);
	printf("block_size : %d\n", block_size);
	printf("associativity : %d\n", assoc);
	if(repl_policy==0)
	printf("replacement policy : %s", LRU);
	else
		printf("replacement policy : %s", RAND);

	printf("cache accesses : %d\n", hit_count + miss_count);
	printf("cache_hits : %d\n", hit_count);
	printf("cache_misses : %d\n", miss_count);
	printf("cache_miss_rate : %.8lf\n",100 *((double)miss_count / (double)(hit_count + miss_count)));




}

