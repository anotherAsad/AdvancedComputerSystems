#include <iostream>
#include <string>
#include <fstream>
#include <vector>

typedef void funcptr_t(const std::vector<unsigned int> &);

class treenode {
  public:
	static int index_count;
	static int distinct_count;
	static int push_count, pop_count;

	static int temp1, temp2, temp3;

	static std::ofstream fstream_compressed;
	static std::string scan_stack;

	static char *compressed_sequence;
	static unsigned c_idx;			// compressed_stream_idx

	std::vector<unsigned int> idx_list;		// will use for black magic
	treenode *subnode[27];

	// methods
	treenode();
	void addElement(std::string word);
	treenode *createNodesFromWord(std::string word);
	treenode *lookup(std::string word);
	treenode *searchElement(std::string word);
	void traverseNode();

	// templated function for print or write scans.
	static funcptr_t *print_or_write;

	// These two functions will be muxed by print_or_write funcptr
	static void print_details(const std::vector<unsigned int> &idx_list);
	static void write_details(const std::vector<unsigned int> &idx_list);
};