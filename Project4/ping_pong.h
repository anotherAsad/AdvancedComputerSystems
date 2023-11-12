class ping_pong {
  public:
	std::mutex ownership;

	int base_idx = 0;

	int filled_idx = 0;					// only filler issues this.
	int read_idx = 0;					// filler sets to zero. processor only increments.

	bool is_empty() {
		return read_idx == filled_idx;
	}

	bool is_full() {
		return filled_idx == (1024*1024);
	}

	std::string elem[1024*1024];

	std::string pop_elem(void) {
		return elem[read_idx++];
	}

	void push_elem(std::string str) {
		elem[filled_idx++] = str;
	}
};