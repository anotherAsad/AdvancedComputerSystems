#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>

#include "tree_class.h"

#define PING_PONG_COUNT 1

std::mutex my_mutex;

class ping_pong {
  public:
	std::mutex ownership;

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

ping_pong ping_pong_arr[PING_PONG_COUNT];

bool halt_threads = false;

// Finds free ping pongs
void AddElemsFromPingPongs(treenode *mother_node) {
	static int break_count = 0;
	// try to find any ping-pong you can
	for(int i=0; i<PING_PONG_COUNT; i = (i+1)%PING_PONG_COUNT) {
		if(halt_threads && break_count++ == 10) {
			break;
		}

		if(ping_pong_arr[i].ownership.try_lock()) {
			// keep feeding while the pan is not empty
			while(!ping_pong_arr[i].is_empty()) {
				mother_node->addElement(ping_pong_arr[i].pop_elem());
			}

			ping_pong_arr[i].ownership.unlock();
		}
	}
}

void FillPingPongs(std::stringstream *uncompressed_stream) {
	std::string read_str;
	// try to find any ping-pong you can
	for(int i=0; i<PING_PONG_COUNT; i = (i+1)%PING_PONG_COUNT) {
		if(halt_threads)
			break;

		if(ping_pong_arr[i].ownership.try_lock()) {
			if(ping_pong_arr[i].is_empty()) {
				// reset flags
				ping_pong_arr[i].filled_idx = ping_pong_arr[i].read_idx = 0; 
				// fill up the ping-pong if it is not full, or no more
				while(!ping_pong_arr[i].is_full()) {
					if(*uncompressed_stream >> read_str) 
						ping_pong_arr[i].push_elem(read_str);
					else {
						halt_threads = true;
						break;
					}
				}
			}
			// unlock and find another ping pong
			ping_pong_arr[i].ownership.unlock();
		}
	}
}

void readFileToString(std::stringstream &uncompressed_stream, const char *file_path){
	std::ifstream fin;

	fin.open(file_path);
	uncompressed_stream << fin.rdbuf();
	fin.close();

	return;
}

int main() {
	std::string read_str;
	std::stringstream uncompressed_stream;

	// Make mother node on the stack
	treenode mother_node;

	treenode *last_node = NULL;
	int est_size_in_mem = 0;

	// Read the entire column into file
	std::cout << "Reading file..." << std::endl;
	readFileToString(uncompressed_stream, "../Column_8Mentries.txt");
	std::cout << "File read complete." << std::endl;

	std::thread t1(AddElemsFromPingPongs, &mother_node);

	FillPingPongs(&uncompressed_stream);
	//std::thread t2(threadedAddElement, &uncompressed_stream, &mother_node);

	t1.join();
	//t2.join();

	est_size_in_mem = treenode::distinct_count*8 + 4*treenode::index_count;

	std::cout << "\tDictionary Encoded\n"
			  << "Distinct Elements: " << treenode::distinct_count << "\n"
			  << "Total Elements: " << treenode::index_count << "\n"
			  << "Total Deltas: " << treenode::index_count - treenode::distinct_count << "\n";

	std::cout << "estimate_size_in_mem: " << (est_size_in_mem >> 20) << " MB\n";

	// REPL
	std::string inp;

	while(true) {
		std::cout << ">> ";
		std::getline(std::cin, inp);

		if(inp == ":q")
			break;
		else if(inp == ":t" || inp == ":w") {
			treenode::scan_stack = "";

			if(last_node == NULL) {
				std::cout << "Last node was empty. Can't traverse.\n";
				continue;
			}

			if(inp == ":w") {
				treenode::compressed_sequence = new char[est_size_in_mem];
				treenode::c_idx = 0;
				treenode::print_or_write = treenode::write_details;
			}
			else {
				treenode::print_or_write = treenode::print_details;
			}

			// Traverse all the nodes to find matches
			last_node->traverseNode();

			if(inp == ":w") {		// insert time here before dumping
				std::cout << "Ratio 16-bit deltas: " << (double)treenode::temp1/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "\n"
						  << "Ratio 24-bit deltas: " << (double)treenode::temp2/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "\n"
						  << "Ratio 32-bit deltas: " << (double)treenode::temp3/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "\n"
						  << "\n Dumping compressed column to file...";

				treenode::fstream_compressed.open("Compressed.txt", std::ios::binary);
				treenode::fstream_compressed.write(treenode::compressed_sequence, treenode::c_idx+1);    
				treenode::fstream_compressed.close();
				
				std::cout << "Compressed column Written." << std::endl;

				delete treenode::compressed_sequence;
			}
		}
		else {
			last_node = mother_node.searchElement(inp);
		}
	}
}