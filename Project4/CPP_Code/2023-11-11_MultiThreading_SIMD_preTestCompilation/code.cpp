#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <immintrin.h>

bool compare_four_simd(std::vector<std::string> str_vec, std::string comp_str) {
	int match_mask, retval = 0;
	// make a sortie of 4 strings of 8 character
	__m256i compare_sortie = _mm256_set_epi64x(
		*(long long *) &str_vec[0][0],
		*(long long *) &str_vec[1][0],
		*(long long *) &str_vec[2][0],
		*(long long *) &str_vec[3][0]
	);

	__m256i base_sortie = _mm256_broadcastq_epi64(*(__m128i *) &comp_str[0]);		// copy the 8 bytes to all

	__m256i cmp_res = _mm256_cmpeq_epi8(compare_sortie, base_sortie);				// compare
	match_mask = _mm256_movemask_epi8(cmp_res);									// condense to bit info

	char *mask_iter = (char *) &match_mask;
	
	retval = (mask_iter[0] == 0xFF) || (mask_iter[1] == 0xFF) || (mask_iter[2] == 0xFF) || (mask_iter[3] == 0xFF);

	return retval;
}

bool compare_four(std::vector<std::string> str_vec, std::string comp_str) {
	int retval = 0;

	for(int i=0; i<4; i++) {
		retval = (comp_str.find(str_vec[i]) != -1);
	}

	return retval;
}

int main() {
	std::string str0, str1, str2, str3, str4;

	str0 = "Hello   ";

	str1 = "Hello   ";
	str2 = "Yello   ";
	str3 = "Hella   ";
	str4 = "Helio   ";

	std::cout << "match_count = " << std::hex << 255 << std::endl;

	return 0;
}