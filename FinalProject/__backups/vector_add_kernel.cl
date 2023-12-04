__kernel void simple_add(__global const int *A, __global const int *B, __global float *C) {
	// Get the index of the current element to be processed
	int i = get_global_id(0);

	// Do the operation
	C[i] = A[i] + B[i];
	
	return;
}

__kernel void euclidean_distance(__global const int *A, __global const int *B, __global float *C) {
	// Get the index of the current element to be processed
	int i = get_global_id(0);

	// Do the operation
	C[i] = sqrt(powr((float)A[i], 2) + powr((float)B[i], 2));
	
	return;
}
