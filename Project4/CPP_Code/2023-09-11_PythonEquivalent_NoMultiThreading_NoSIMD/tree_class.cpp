#include "tree_class.h"

int treenode::index_count = 0;
int treenode::distinct_count = 0;
int treenode::push_count = 0;
int treenode::pop_count = 0;

int treenode::temp1=0, treenode::temp2=0, treenode::temp3=0;

std::ofstream treenode::fstream_compressed;
std::string treenode::scan_stack;

char *treenode::compressed_sequence = NULL;
unsigned treenode::c_idx = 0;

funcptr_t *treenode::print_or_write = NULL;

treenode::treenode() {
	// initialize subnodes with NUKK
	for(int i=0; i<27; i++)
		this->subnode[i] = NULL;

	return;
}

void treenode::addElement(std::string word) {
	static char key;

	if(word.length() == 0) {					// This must be the final node
		treenode::index_count++;
		// Keep track of distincts. Allocate more space to vector in anticipation
		if(this->idx_list.empty()) {
			treenode::distinct_count++;
			this->idx_list.reserve(256);
		}

		// add the index to the vector
		this->idx_list.push_back(index_count);

		return;
	}

	// For non-final nodes.
	key = word[0] & 0x1F;

	// If the subnode does not exist, create it
	if(this->subnode[key] == NULL)
		this->subnode[key] = new treenode();

	this->subnode[key]->addElement(word.substr(1));
}

treenode *treenode::createNodesFromWord(std::string word) {
	static char key;
	// if the word has been encoded add in this sequence, add the index tag now.
	if(word.length() == 0)
		return this;

	// If there is still some of the word to be enqueued ...
	key = word[0] & 0x1F;
	
	// If the subnode does not exist, create it
	if(this->subnode[key] == NULL)
		this->subnode[key] = new treenode();

	// Keep enqueuing the word
	return this->subnode[key]->createNodesFromWord(word.substr(1));
}

treenode *treenode::lookup(std::string word) {
	static char key;
	
	if(word.length() == 0)			// if node match is complete
		return this;

	// Keep looking deeper
	key = word[0] & 0x1F;

	if(this->subnode[key] == NULL)
		return NULL;

	return this->subnode[key]->lookup(word.substr(1));
}

treenode *treenode::searchElement(std::string word) {
	treenode *returned_node = this->lookup(word);

	if(returned_node == NULL)
		std::cout << "Word not found...";
	else {
		if(!returned_node->idx_list.empty()) {		// if not empty
			std::cout << "Word found at indices:\n";

			for(auto elem : returned_node->idx_list) 
  				std::cout << elem << ", ";
		}
		else {
			std::cout << word << " is a prefix.";
		}
	}

	std::cout << std::endl;

	return returned_node;
}

void treenode::traverseNode() {
	if(! this->idx_list.empty())
		print_or_write(this->idx_list);

	// go deeper
	for(int i=0; i<27; i++) {
		if(this->subnode[i] != NULL) {
			scan_stack.push_back(char(96+i));
			this->subnode[i]->traverseNode();
			scan_stack.pop_back();
		}
	}

	return;
}

void treenode::print_details(const std::vector<unsigned int> &idx_list) {
	std::cout << scan_stack << ": ";

	for (auto elem : idx_list) {
		std::cout << elem << ", ";
	}

	std::cout << "\n";

	return;
}

// format [1byte seqLength, N byte seq, 2 byte idx_list_length, 4 byte first idx, N byte compressed delta stream]
void treenode::write_details(const std::vector<unsigned int> &idx_list) {
	static unsigned len_var, delta;

	len_var = scan_stack.length();

	for(int i=0; i<len_var; i++)					// N byte sequence
		compressed_sequence[c_idx++] = scan_stack[i];		

	len_var = idx_list.size();	

	// 2 bytes for idx count. MSnibble is indicator for idx start
	*((unsigned short *) &compressed_sequence[c_idx]) = (unsigned short) (len_var | 0xF000);
	c_idx += 2;

	*((unsigned int *) &compressed_sequence[c_idx]) = (unsigned int) idx_list[0];
	c_idx += 4;

	for(int i=1; i<len_var; i++) {
		delta = idx_list[i] -  idx_list[i-1];

		// huffman coding
		if((delta & 0xFFFFC000) == 0) {			// Can put in 14 bits? [2'b10, 14'hXXXX]
			temp1++;
			*((unsigned short *) &compressed_sequence[c_idx]) = (unsigned short) (delta | 0x8000);
			c_idx += 2;
		}
		else if((delta & 0xFF800000) == 0) {		// Can put in 23 bits? [1'b0, 23'hXXXXXX]
			temp2++;
			*((unsigned int *) &compressed_sequence[c_idx]) = delta;
			c_idx += 3;
		}
		else {								// Fit in 30 bits. [2'b11, 30'hXXXXXXXX]
			temp3++;
			*((unsigned int *) &compressed_sequence[c_idx]) = delta | 0xC0000000;
			c_idx += 4;
		}
	}

	return;
}
