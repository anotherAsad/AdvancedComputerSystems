#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <x86intrin.h>
#include <time.h>
#include <string.h>
#include <linux/perf_event.h>

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

#define PAGE_BASE		(0x00000000)			// The physical address to be mapped by mmap
#define PAGE_SIZE		(0x00001000)			// 4096 in decimal. Single RAM page size. Effects TLB.
#define L1_SIZE			(0x00008000)			// 32K in decimal.
#define L2_SIZE			(0x00040000)			// 256K in decimal.
#define L3_SIZE			(0x00600000)			// 6291456 in decimal. Equals 6 MB.
#define MEM_BUF_SIZE	(0x08000000)			//   128 MB

unsigned char *mappedMem;						// Pointer to mapped memory

char target_name[][3]  = {"L1", "L2", "L3", "RAM"};
unsigned target_size[] = {L1_SIZE, L2_SIZE, L3_SIZE, MEM_BUF_SIZE};


// Fills the Arr with values from 0 to TargetMemorySize/4-1, but in random order. Done to beat prefetchers.
void InitializeArray(unsigned *Arr, unsigned TargetMemorySize) {
	unsigned mask = ((TargetMemorySize>>2)-1);
	unsigned temp = 0;
	unsigned rand_addr = 0;

	for(int i=0; i<TargetMemorySize>>2; i++)
		Arr[i] = i;
	
	for(int i=0; i<TargetMemorySize>>2; i++) {
		rand_addr = rand() & mask;
		// swap the random and linear addressed values
		temp = Arr[i];
		Arr[i] = Arr[rand_addr];
		Arr[rand_addr] = temp;
	}
	
	return;
}

unsigned long long selfVectorProduct(unsigned *Arr, unsigned IterCount) {
	unsigned addr1, addr2;
	unsigned long long sum = 0;

	addr1 = 0x0;
	addr2 = 0xF;

	for(int i=0; i<IterCount>>2; i++) {
		addr1 = Arr[addr1];
		addr2 = Arr[addr2];
		sum += Arr[addr1] * Arr[addr2];
	}

	return sum;
}

double timedExecution(unsigned TargetMemorySize) {
	unsigned long long sum;
	int i;
	
	for(i=0; i<4; i++) {
		if(TargetMemorySize != target_size[i]) {
			printf("[ ] " ANSI_COLOR_RED "populating %s buffer..." ANSI_COLOR_RESET "\n", target_name[i]);
			InitializeArray((unsigned *)mappedMem, target_size[i]);
		}
	}
	
	for(i=0; i<4; i++) {
		if(TargetMemorySize == target_size[i]) {
			printf("[*] " ANSI_COLOR_GREEN "populating %s buffer..." ANSI_COLOR_RESET "\n\n", target_name[i]);
			InitializeArray((unsigned *)mappedMem, target_size[i]);
		}
	}		

	clock_t begin = clock();

	sum = selfVectorProduct((unsigned *)mappedMem, MEM_BUF_SIZE*16L);
	
	clock_t end = clock();

	return (double)(end - begin)/CLOCKS_PER_SEC;
}

int main(int argc, char *argv[]) {
	double L1_time, L2_time, L3_time, RAM_time;
	mappedMem = mmap(NULL, MEM_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	srand(time(NULL));   // Initialization, should only be called once.

	printf("\n\t\t\t*** EXCUTION FLOW STARTS ***\n\n");	
	
	for(int i=0; i<4; i++) {
		if(argc == 1 || !strcmp(argv[1], target_name[i])) {
			// L1-D time
			printf("Targeting %s...\nActive Buffer Size = %u KB\n", target_name[i], target_size[i]>>10);
			printf("Total operations counted for timing: 512*1024*1024 = " ANSI_BGCLR_GREEN ANSI_COLOR_BLACK "~536 Million" ANSI_COLOR_RESET "\n\n");
			L1_time = timedExecution(target_size[i]);
			printf("Total Execution time when targeting %s: " ANSI_BGCLR_GREEN ANSI_COLOR_BLACK "%lf seconds" ANSI_COLOR_RESET "\n\n", target_name[i], L1_time);
		}
	}
	
	printf("\t\t\t*** EXCUTION FLOW ENDS. PERF REPORT SHOULD FOLLOW ***\n\n");
	
	munmap(mappedMem, MEM_BUF_SIZE);
	return 0;
}

