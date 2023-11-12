#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>

#include "tree_class.h"
#include "unique_elem_stream.h"
#include "ping_pong.h"

#define PING_PONG_COUNT 3

ping_pong ping_pong_arr[PING_PONG_COUNT];

bool halt_threads = false;

// Finds free ping pongs
void AddElemsFromPingPongs(treenode *mother_node, int *distinct_count) {
	int break_count = 0;
	
	*distinct_count = 0;
	// try to find any ping-pong you can
	for(int i=0; i<PING_PONG_COUNT; i = (i+1)%PING_PONG_COUNT) {
		if(halt_threads && (break_count++ == PING_PONG_COUNT)) {
			break;
		}

		if(ping_pong_arr[i].ownership.try_lock()) {
			// keep feeding while the pan is not empty
			while(!ping_pong_arr[i].is_empty()) {
				*distinct_count += mother_node->addElement(ping_pong_arr[i].pop_elem(), ping_pong_arr[i].base_idx++);
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
				ping_pong_arr[i].base_idx = treenode::index_count;
				// fill up the ping-pong if it is not full
				while(!ping_pong_arr[i].is_full()) {
					if(*uncompressed_stream >> read_str) {
						treenode::index_count++;
						ping_pong_arr[i].push_elem(read_str);
					}
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
	readFileToString(uncompressed_stream, "../Column.txt");
	std::cout << "File read complete.\n" << std::endl;

	int thread_count = PING_PONG_COUNT - 2;
	std::thread *threadArr[thread_count];
	int distinct_count_arr[thread_count];

	for(int i=0; i<thread_count; i++) {
		threadArr[i] = new std::thread(AddElemsFromPingPongs, &mother_node, &distinct_count_arr[i]);
	}

	std::cout << "Encoding Dictionary with " << thread_count << " threads..." << std::endl;

	auto start_time = std::chrono::high_resolution_clock::now();

	FillPingPongs(&uncompressed_stream);

	for(int i=0; i<thread_count; i++) {
		threadArr[i]->join();
		delete threadArr[i];
		treenode::distinct_count += distinct_count_arr[i];
	}

	auto stop_time = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	std::cout << "\tDictionary Encoded.\n"
			  << "Time Taken: " <<  duration.count()/1000.0 << " seconds.\n" << std::endl;

	est_size_in_mem = treenode::distinct_count*8 + 4*treenode::index_count;

	std::cout << "Distinct Elements: " << treenode::distinct_count << "\n"
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
		else if(inp == ":g") {				// unique_elem search requested search requested
			std::cout << "You are hitting a unique_elem_search menu without SIMD...\n";
			std::cout << "s> ";
			std::getline(std::cin, inp);

			int res = unique_elem_stream_t::search_unique_elem_stream(inp, false);

			std::cout << "Found at idx: " << res << std::endl;
		}
		else if(inp == ":h") {				// unique_elem search requested search requested
			std::cout << "You are hitting a unique_elem_search menu with SIMD...\n";
			std::cout << "s> ";
			std::getline(std::cin, inp);

			int res = unique_elem_stream_t::search_unique_elem_stream(inp, true);

			std::cout << "Found at idx: " << res << std::endl;
		}
		else if(inp == ":t" || inp == ":w" || inp == ":s") {
			treenode::scan_stack = "";

			if(last_node == NULL) {
				std::cout << "Last node was empty. Can't traverse.\n";
				continue;
			}

			if(inp == ":w") {
				treenode::compressed_sequence = new char[est_size_in_mem];
				treenode::c_idx = 0;
				treenode::print_or_write = treenode::write_details_huffman;
			}
			else if(inp == ":s") {
				treenode::print_or_write = treenode::save_details;
			}
			else {
				treenode::print_or_write = treenode::print_details;
			}

			// Traverse all the nodes to find matches
			last_node->traverseNode();

			if(inp == ":w") {		// insert time here before dumping
				std::cout << "Ratio 16-bit deltas: " << (double) treenode::temp1/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "\n"
						  << "Ratio 24-bit deltas: " << (double) treenode::temp2/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "\n"
						  << "Ratio 32-bit deltas: " << (double) treenode::temp3/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "\n"
						  << "\n Dumping compressed column to file...\n";

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