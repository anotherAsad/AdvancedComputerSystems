#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>

#include "tree_class.h"
#include "unique_elem_stream.h"
#include "ping_pong.h"

#define PING_PONG_COUNT 10

#define BASELINE_TEST 0
#define ULTRA_TEST 0
#define SIMD_TEST 0

ping_pong ping_pong_arr[PING_PONG_COUNT];

bool halt_threads = false;

extern unique_elem_stream_t unique_elem_stream;

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

void readFileToStringStream(std::stringstream &uncompressed_stream, const char *file_path) {
	std::ifstream fin;

	fin.open(file_path);
	uncompressed_stream << fin.rdbuf();
	fin.close();

	return;
}

std::string readFileToString(const char *file_path) {
	std::stringstream uncompressed_stream;
	std::ifstream fin;

	fin.open(file_path);
	uncompressed_stream << fin.rdbuf();
	fin.close();

	return uncompressed_stream.str();
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
	readFileToStringStream(uncompressed_stream, "./Column.txt");
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

	std::cout << "estimate_size_in_mem: " << (est_size_in_mem >> 20) << " MB\n" << std::endl;

	uncompressed_stream.seekg(0);

	// word search experiments
	treenode::print_or_write = treenode::save_details;

	start_time = std::chrono::high_resolution_clock::now();
	mother_node.traverseNode();
	stop_time = std::chrono::high_resolution_clock::now();

	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	std::cout << "Traversed tree to create unique elements dictionary.\n"
			  << "Time taken: " << duration.count() << " milliseconds\n"
			  << std::endl;



	if(BASELINE_TEST) {
		std::cout << "\t*** Baseline search test. Will query 1 Thousand entries... ***\n" << std::endl;

		std::string read_str;

		start_time = std::chrono::high_resolution_clock::now();
		// unique_elem_stream.unique_elem to be iterated for available values
		for(int i=0; i < 1000; i++) {
			uncompressed_stream.clear();
			uncompressed_stream.seekg(0, uncompressed_stream.beg);
			// search anew
			while(uncompressed_stream >> read_str) {
				unsigned idx = rand() & 0x7FFFF;		// 2^19 = 512K entries. less than available unique entries
				if(unique_elem_stream.unique_elem[idx].find(read_str) == 0)
					break;
			}
		}

		stop_time = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

		std::cout << "Time taken: " << duration.count()/1000.0 << " seconds for 1 Thousand queries\n"
				<< "Average time per query: " << duration.count()/1000.0 << " milli seconds\n"
				<< std::endl;
	}

	if(ULTRA_TEST) {
		std::cout << "\t*** Silent search test. Will query 10 Million entries... ***\n" << std::endl;
		
		start_time = std::chrono::high_resolution_clock::now();
		// unique_elem_stream.unique_elem to be iterated for available values
		for(int i=0; i < 10000000; i++) {
			unsigned idx = rand() & 0x7FFFF;		// 2^19 = 512K entries. less than available unique entries
			treenode *lastnode = mother_node.lookup(unique_elem_stream.unique_elem[idx]);
		}

		stop_time = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);
		auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);

		std::cout << "Time taken: " << duration.count()/1000.0 << " seconds for 10 Million queries\n"
				  << "Average time per query: " << duration_ns.count()/10000000.0 << " nano seconds\n"
				  << std::endl;

		std::cout << "\t *** Silent prefix scan test. Will scan 10 Million entries... ***\n" << std::endl;
		
		treenode::print_or_write = treenode::save_details;

		start_time = std::chrono::high_resolution_clock::now();
		// unique_elem_stream.unique_elem to be iterated for available values
		for(int i=0; i < 10000000; i++) {
			unsigned idx = rand() & 0x7FFFF;		// 2^26 = 64M entries. less than available 128M entries
			treenode *lastnode = mother_node.lookup(unique_elem_stream.unique_elem[idx]);
			lastnode->traverseNode();
		}

		stop_time = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);
		duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);


		std::cout << "Time taken: " << duration.count()/1000.0 << " seconds for 10 Million queries\n"
				<< "Average time per query: " << duration_ns.count()/10000000.0 << " nano seconds\n"
				<< std::endl;
	}

	if(SIMD_TEST) {
		std::cout << "\t *** Compressed stream query/prefix test [NO SIMD]. Will scan 10 thousand entries... ***\n" << std::endl;

		start_time = std::chrono::high_resolution_clock::now();
		// unique_elem_stream.unique_elem to be iterated for available values
		for(int i=0; i < 10000; i++) {
			unsigned idx = rand() & 0x7FFFF;		// 2^26 = 64M entries. less than available 128M entries
			int res = unique_elem_stream_t::search_unique_elem_stream(unique_elem_stream.unique_elem[idx], false);
		}

		stop_time = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);


		std::cout << "Time taken: " << duration.count()/1000.0 << " seconds for 10 Thousand queries\n"
				<< "Average time per query: " << duration.count()/10000.0 << " milliseconds\n"
				<< std::endl;

		std::cout << "\t *** Compressed stream query/prefix test [SIMD]. Will scan 10 thousand entries... ***\n" << std::endl;

		start_time = std::chrono::high_resolution_clock::now();
		// unique_elem_stream.unique_elem to be iterated for available values
		for(int i=0; i < 10000; i++) {
			unsigned idx = rand() & 0x7FFFF;		// 2^26 = 64M entries. less than available 128M entries
			int res = unique_elem_stream_t::search_unique_elem_stream(unique_elem_stream.unique_elem[idx], true);
		}

		stop_time = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);


		std::cout << "Time taken: " << duration.count()/1000.0 << " seconds for 10 Thousand queries\n"
				<< "Average time per query: " << duration.count()/10000.0 << " milliseconds\n"
				<< std::endl;
		
	}

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
			std::string det_mode;
			treenode::scan_stack = "";

			if(last_node == NULL) {
				std::cout << "Last node was empty. Can't traverse.\n";
				continue;
			}

			if(inp == ":w") {
				treenode::compressed_sequence = new char[est_size_in_mem];
				treenode::c_idx = 0;
				treenode::print_or_write = treenode::write_details_huffman;

				det_mode = "written to memory in encoded, integer-compressed, file-ready format.";
			}
			else if(inp == ":s") {
				treenode::print_or_write = treenode::save_details;

				det_mode = "written in encoded, unique-word/indeices pair format file-ready format.";
			}
			else {
				treenode::print_or_write = treenode::print_details;
				det_mode = "printed on screen.";
			}

			std::cout << "Traversing all subnodes of the last node.... "
					  << "This equals to searching for all children of a given prefix.\n"
					  << "Details will be " << det_mode << std::endl;

			auto start_time = std::chrono::high_resolution_clock::now();
			// Traverse all the nodes to find matches
			last_node->traverseNode();
			auto stop_time = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

			std::cout << "Time Taken: " <<  duration.count() << " milli-seconds.\n" << std::endl;

			if(inp == ":w") {		// insert time here before dumping
				std::cout << "Percentage 16-bit delta & huffman-coded ints: " << (double) treenode::temp1/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "%\n"
						  << "Percentage 24-bit delta & huffman-coded ints: " << (double) treenode::temp2/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "%\n"
						  << "Percentage 32-bit delta & huffman-coded ints: " << (double) treenode::temp3/(treenode::temp1 + treenode::temp2 + treenode::temp3) << "%\n"
						  << "\n\nDumping compressed column to file...\n";

				treenode::fstream_compressed.open("Compressed.txt", std::ios::binary);
				treenode::fstream_compressed.write(treenode::compressed_sequence, treenode::c_idx+1);    
				treenode::fstream_compressed.close();
				
				std::cout << "Compressed column Written." << std::endl;

				delete treenode::compressed_sequence;
			}
		}
		else {
			if(inp == "")
				std::cout << "\nmother node prepared for traversal.\n";

			auto start_time = std::chrono::high_resolution_clock::now();
			last_node = mother_node.searchElement(inp);
			auto stop_time = std::chrono::high_resolution_clock::now();


			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);

			std::cout << "\nTime Taken: " <<  duration.count() << " nano-seconds.\n" << std::endl;
		}
	}
}