#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_BLACK   "\x1b[30m"
#define ANSI_BGCLR_BLUE    "\x1b[104m"
#define ANSI_BGCLR_GREEN   "\x1b[42m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_BLACK_ON_GREEN ANSI_BGCLR_GREEN ANSI_COLOR_BLACK

#define PAGE_BASE				(0x00000000)					// Artefact only. No effect.
#define PAGE_SIZE				(0x00001000)					// 4096 in decimal. Single RAM page size. Effects TLB.
#define PAGE_COUNT_INTRA		576UL							// Total number of pages in intra page memory access experiment
#define PAGE_COUNT_INTER		1024UL * PAGE_COUNT_INTRA		// Total number of pages in inter page memory access experiment

#define STUPID_BREAKPOINT printf("Here\n");fflush(stdout);

// Fills the Arr with values from 0 to TargetMemorySize/4-1, but in random order. Done to beat prefetchers.
void InitializePages(unsigned *test_ptr, unsigned vec_size) {
	unsigned temp, rand_addr;
	unsigned addr_sf = vec_size/PAGE_COUNT_INTRA;			// addr_sf is address scale factor. either 1 or 1024.
	unsigned mask = vec_size-1;

	for(unsigned i=0; i < PAGE_COUNT_INTER; i++)
		test_ptr[addr_sf * i] = addr_sf * i;
	
	for(unsigned i=0; i < PAGE_COUNT_INTER; i++) {
		rand_addr = rand() & mask;
		// swap the random and linear addressed values
		temp = test_ptr[addr_sf * i];
		test_ptr[addr_sf * i] = test_ptr[rand_addr * addr_sf];
		test_ptr[rand_addr * addr_sf] = temp;
	}
	
	return;
}

unsigned long selfVectorProduct(unsigned *test_ptr, unsigned inter_max, unsigned vec_size) {
	unsigned addr1, addr2;
	unsigned addr_sf = vec_size/PAGE_COUNT_INTRA;
	unsigned long sum = 0;

	addr1 = 0x0;
	addr2 = 0xF;

	for(int i=0; i < inter_max; i++) {
		addr1 = test_ptr[addr1];
		addr2 = test_ptr[addr2];
		sum += test_ptr[addr1] * test_ptr[addr2];
	}

	return sum;
}

double timedExecution(unsigned *test_ptr, unsigned PageCount) {
	unsigned long sum;
	int i;
	
	InitializePages(test_ptr, PageCount);
	
	clock_t begin = clock();

	selfVectorProduct(test_ptr, PAGE_SIZE*3000, PageCount);
	
	clock_t end = clock();

	return (double)(end - begin)/CLOCKS_PER_SEC;
}

int main(int argc, char *argv[]) {
	unsigned *intra_page_ptr;				// Will be used to access data local to a PAGE_SIZE
	unsigned *inter_page_ptr;				// Will be used to access data with access granularity explicitly greater than PAGE_SIZE
	double intra_page_case_time, inter_page_case_time;

	srand(time(NULL));   // Seed Initialization, should only be called once.

	
	// Allocate memory for a single page (intra)
	intra_page_ptr = mmap(NULL, PAGE_SIZE*PAGE_COUNT_INTRA, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	// Allocate memory for multiple pages (inter)
	inter_page_ptr = mmap(NULL, PAGE_SIZE*PAGE_COUNT_INTER, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);		// 1024 pages more than above
	
	printf("\n\t\t\t*** EXCUTION FLOW STARTS ***\n\n");
	
	if(argc == 1 || !strcmp(argv[1], "intra_page")) {
		intra_page_case_time = timedExecution(intra_page_ptr, PAGE_COUNT_INTRA);
		printf("Time for Algorithm performing memory accesses within a single page   :" ANSI_COLOR_BLACK_ON_GREEN "%lf seconds" ANSI_COLOR_RESET "\n\n", intra_page_case_time);
	}
	
	if(argc == 1 || !strcmp(argv[1], "inter_page")) {
		inter_page_case_time = timedExecution(inter_page_ptr, PAGE_COUNT_INTER);
		printf("Time for Algorithm performing memory accesses accross multiple pages : " ANSI_COLOR_BLACK_ON_GREEN "%lf seconds" ANSI_COLOR_RESET "\n\n", inter_page_case_time);
	}
	
	printf("\t\t\t*** EXCUTION FLOW ENDS. PERF REPORT SHOULD FOLLOW ***\n\n");
	
	// unmap continuous pages
	munmap(intra_page_ptr, PAGE_SIZE * PAGE_COUNT_INTRA);	
	munmap(inter_page_ptr, PAGE_SIZE * PAGE_COUNT_INTER);
	
	return 0;
}

