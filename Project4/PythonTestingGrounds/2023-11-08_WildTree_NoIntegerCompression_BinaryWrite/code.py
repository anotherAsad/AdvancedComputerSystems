#! /usr/bin/python3

import tree_class

total_len = 0
motherNode = tree_class.NodeClass()

file1 = open("Column_4Mentries.txt", "r")

for _ in range(0, 4*1024*1024):
	entry = next(file1)
	total_len += len(entry[:-1])
	motherNode.addElement(entry[:-1])

file1.close()

print("Dictionary created.")

if False:
	file1 = open("Column_4Mentries.txt", "r")

	for _ in range(0, 1024*1024):
		entry = next(file1)
		motherNode.searchElement(entry[:-1])

NodeCount = tree_class.NodeClass.node_count + tree_class.NodeClass.index_count


print("Node Count:\t", NodeCount)
print("Total len:\t", total_len)

print(f"Compression Ratio: {total_len/NodeCount:0.3f}")
print(f"Post Compression Size of Original: {NodeCount/total_len*100:0.3f} %")

print("Lookup REPL...")

last_node = None

while True:
	inp = input(">> ")
	if inp == ":q":
		break
	elif inp == ":t" or inp == ":w":
		if last_node == None:
			print("Last node was empty. Can't traverse.")
		else:
			tree_class.NodeClass.scan_stack.clear()
			tree_class.NodeClass.pop_count = 0
			# Choose between print or write
			if inp == ":w":
				tree_class.NodeClass.file_ptr = open("Compressed.txt", "wb")
				tree_class.NodeClass.print_or_write = tree_class.NodeClass.write_details
			else:
				tree_class.NodeClass.print_or_write = tree_class.NodeClass.print_details
			# Traverse node
			last_node.traverseNode()
			# close file if open
			tree_class.NodeClass.file_ptr.close() if inp == ":w" else None
			print()
	else:
		last_node = motherNode.searchElement(inp)
	
	


