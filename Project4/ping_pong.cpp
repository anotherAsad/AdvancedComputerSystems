#include <iostream>
#include <string>
#include <thread>
#include <mutex>

class ping_pong {
  public:
	std::mutex ownership;

	int filled_idx = 0;					// only filler issues this.
	int read_idx = 0;					// filler sets to zero. processor only increments.

	bool is_empty() {
		return read_idx == filled_idx;
	}

	bool is_full() {
		return (filled_idx == 1024*1024);
	}

	std::string elem[1024*1024];

	std::string pop_elem(void) {
		return elem[read_idx++];
	}

	void push_elem(std::string str) {
		elem[filled_idx++] = str;
	}
};

ping_pong ping_pong_arr[3];

void readerFunction() {
	// try to find any ping-pong you can
	for(int i=0; i<3; i = (i+1)%3) {
		if(ping_pong_arr[i].ownership.try_lock()) {
			// keep feeding while the pan is not empty
			while(!ping_pong_arr[i].is_empty()) {
				std::cout << ping_pong_arr[i].pop_elem();
			}

			ping_pong_arr[i].ownership.unlock();
		}
	}
}

void fillerFunction() {
	// try to find any ping-pong you can
	for(int i=0; i<3; i = (i+1)%3) {
		if(ping_pong_arr[i].ownership.try_lock()) {
			// keep feeding while the pan is not empty
			if(ping_pong_arr[i].is_empty()) {
				while(!is_full() || std::cin >> )
				std::cout << ping_pong_arr[i].pop_elem();
			}

			ping_pong_arr[i].ownership.unlock();
		}
	}
}