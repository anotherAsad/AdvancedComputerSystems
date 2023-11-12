struct unique_elem_stream_t {
	std::vector<std::string> unique_elem;
	std::vector<std::vector<unsigned int> *> idx_list_ptrs;		// vector or pointers to vector lists

	static int search_unique_elem_stream(std::string word, bool simd);
};
