#include <vector>
#include <string>
#include <immintrin.h>

#include "unique_elem_stream.h"

unique_elem_stream_t unique_elem_stream;

typedef bool cmp_func_t(std::vector<std::string> &, std::string, int);

bool compare_four(std::vector<std::string> &str_vec, std::string comp_str, int base_idx);
bool compare_four_simd(std::vector<std::string> &str_vec, std::string comp_str, int base_idx);

int unique_elem_stream_t::search_unique_elem_stream(std::string word, bool simd) {
	std::vector<unsigned int> indices_vec;
	int idx;
	bool comp_pass = false;
	std::vector<std::string> &str_vec = unique_elem_stream.unique_elem;

	cmp_func_t *cmp_func;

	if(simd)
		cmp_func = compare_four_simd;
	else
		cmp_func = compare_four;
	
	for(idx = 0; idx < str_vec.size()/4; idx += 4) {
		comp_pass = cmp_func(str_vec, word, idx);

		if(comp_pass)
			break;
	}

	// this is genuinely clever: resets the idx in such a way that the `for loop` that follows can seek the right index.
	if(comp_pass) {
		comp_pass = false;
		idx -= 4;
	}
	
	for(idx; !comp_pass && idx < str_vec.size(); idx++) {
		comp_pass = (word.find(str_vec[idx]) != -1);
	}

	return idx;
}

bool compare_four_simd(std::vector<std::string> &str_vec, std::string comp_str, int base_idx) {
	int match_mask, retval = 0;
	// make a sortie of 4 strings of 8 character
	__m256i compare_sortie = _mm256_set_epi64x(
		*(long long *) &str_vec[base_idx+0][0],
		*(long long *) &str_vec[base_idx+1][0],
		*(long long *) &str_vec[base_idx+2][0],
		*(long long *) &str_vec[base_idx+3][0]
	);

	__m256i base_sortie = _mm256_broadcastq_epi64(*(__m128i *) &comp_str[0]);		// copy the 8 bytes to all

	__m256i cmp_res = _mm256_cmpeq_epi8(compare_sortie, base_sortie);				// compare
	match_mask = _mm256_movemask_epi8(cmp_res);									// condense to bit info

	char *mask_iter = (char *) &match_mask;
	
	retval = (mask_iter[0] == 0xFF) || (mask_iter[1] == 0xFF) || (mask_iter[2] == 0xFF) || (mask_iter[3] == 0xFF);

	return retval;
}

bool compare_four(std::vector<std::string> &str_vec, std::string comp_str, int base_idx) {
	int retval = 0;

	for(int i=0; i<4; i++) {
		retval = (comp_str.find(str_vec[base_idx+i]) != -1);
	}

	return retval;
}