#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <x86intrin.h>
#include <time.h>

#define PAGE_BASE		(0x00000000)			// The physical address to be mapped by mmap
#define PAGE_SIZE		(0x00001000)			// 4096 in decimal. Single RAM page size. Effects TLB.
#define L1_SIZE			(0x00008000)			// 32K in decimal.
#define L2_SIZE			(0x00040000)			// 256K in decimal.
#define L3_SIZE			(0x00600000)			// 6291456 in decimal. Equals 6 MB.
#define MEM_BUF_SIZE	(0x07800000)			// Equals 128 MB

// Rollover Masks
#define MASK_32K	0x00007FFF					// Address Mask for L1-D
#define MASK_256K	0x0003FFFF					// Address Mask for L2
#define MASK_6M		0x005FFFFF					// Address Mask for L3
#define MASK_MEM	MEM_BUF_SIZE-1

// Address scrambling keys. Pseudo Random address access used to beat pre-fetcher, and cause more frequent misses if needed.
#define RAND_KEY	0xAAAAAAAA

unsigned char *mappedMem;							// Pointer to mapped memory

void InitializeArray(long long *Arr, unsigned TargetMemorySize) {
	for(int i=0; i<TargetMemorySize/8; i++)
		Arr[i] = (rand() << 32 + rand()) & (TargetMemorySize-1);

	return;
}

void vectorProduct(long long *Arr, int IterCount) {
	unsigned addr1, addr2;
	long long sum = 0;

	addr1 = addr2 = 0;

	for(int i=0; i<IterCount/8; i++) {
		addr1 = Arr[addr1];
		addr2 = Arr[addr2];
		sum += Arr[addr1] * Arr[addr2];
	}
}

double timedExection(unsigned TargetMemorySize) {
	InitializeArray((long long *)mappedMem, TargetMemorySize);

	clock_t begin = clock();

	vectorProduct((long long *)mappedMem, MEM_BUF_SIZE*1);

	clock_t end = clock();
	return (double)(end - begin) / CLOCKS_PER_SEC;
}

int main() {
	double L1_time, L2_time, L3_time, RAM_time;
	mappedMem = mmap(NULL, MEM_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, PAGE_BASE);
	srand(time(NULL));   // Initialization, should only be called once.


	printf("Mapped Mem Address: 0x%X\n", mappedMem);


	// L1-D time
	L1_time = timedExection(L1_SIZE);
	printf("Cycles Taken by L1-D: %f\n", L1_time);


	// L2 time
	L2_time = timedExection(L2_SIZE);
	printf("Cycles Taken by L2  : %f\n", L2_time);

	// L3 time
	L3_time = timedExection(L3_SIZE);
	printf("Cycles Taken by L3  : %f\n", L3_time);

	// RAM time
	RAM_time = timedExection(MEM_BUF_SIZE);
	printf("Cycles Taken by RAM : %f\n", RAM_time);

	munmap(mappedMem, MEM_BUF_SIZE);
}