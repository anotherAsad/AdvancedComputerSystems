#include <stdio.h>


int *fmain() {
	int A = 55;
	int *ptr = &A;
	
	return ptr;
}

int main() {
	int *Aptr = fmain();
	
	printf("0x%lX\n", (unsigned long) Aptr);
	return 0;
}
