__kernel void simple_add(__global const float *A, __global const float *B, __global float *C) {
	// Get the index of the current element to be processed
	int i = get_global_id(0);

	// Do the operation
	C[i] = A[i] + B[i];
	
	return;
}

__kernel void euclidean_distance(__global const float *A, __global const float *B, __global float *C) {
	// Get the index of the current element to be processed
	int i = get_global_id(0);

	// Do the operation
	C[i] = sqrt(powr((float)A[i], 2) + powr((float)B[i], 2));
	
	return;
}

__kernel void unit_sphere(__global const float *A, __global const float *B, __global float *C) {
	// Get the index of the current element to be processed
	int i = get_global_id(0);

	// Do the operation
	C[i] = sqrt(powr(1.0, 2)  - powr(fabs(A[i]), 2) - powr(fabs(B[i]), 2));
	
	return;
}
