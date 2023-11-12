#include <iostream>
#include <sstream>

#include "tree_class.h"

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

	treenode mother_node;
	treenode *last_node = NULL;
	int est_size_in_mem = 0;

	// Read the entire column into file
	std::cout << "Reading file..." << std::endl;
	readFileToString(uncompressed_stream, "../Column.txt");
	std::cout << "File read complete." << std::endl;
	
	// make mother node. Is on stack.

	// Encode the entire file
	while(uncompressed_stream >> read_str) {
		mother_node.addElement(read_str);
	}

	est_size_in_mem = treenode::distinct_count*8 + 3*treenode::index_count + treenode::distinct_count;

	std::cout << "Distinct Elements: " << treenode::distinct_count << "\n"
			  << "Total Elements: " << treenode::index_count << "\n"
			  << "Total Deltas: " << treenode::index_count - treenode::distinct_count << "\n";

	std::cout << "estimate_size_in_mem: " << (est_size_in_mem >> 20) << " MB\n";

	// repl
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
				treenode::compressed_sequence = new char[4*treenode::index_count];
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