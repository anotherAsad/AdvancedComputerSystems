#! /usr/bin/python3

from tree_class import *

total_len = 0
motherNode = NodeClass()

file1 = open("Column.txt", "r")

"""
for _ in range(0, 8*1024*1024):
	entry = next(file1)
	total_len += len(entry[:-1])
	motherNode.addElement(entry[:-1])
"""

for entry in file1:
	total_len += len(entry[:-1])
	motherNode.addElement(entry[:-1])

file1.close()

print("Dictionary created.")
print(f"Distinct Elements: {NodeClass.distinct_count}")
print(f"Total Elements: {NodeClass.index_count}")
print(f"Total Deltas: {NodeClass.index_count - NodeClass.distinct_count}")
print(f"Total Nodes: {NodeClass.node_count}")

if False:
	file1 = open("Column_4Mentries.txt", "r")

	for _ in range(0, 1024*1024):
		entry = next(file1)
		motherNode.searchElement(entry[:-1])

uniques_elems = NodeClass.distinct_count*8 + 3*NodeClass.index_count

print("Unique Count:\t", uniques_elems)
print("Total len:\t", total_len)

print(f"Compression Ratio: North of {total_len/uniques_elems:0.3f}")
print(f"Post Compression Size of Original: North of {uniques_elems/total_len*100:0.3f} percent")

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
			NodeClass.scan_stack.clear()
			NodeClass.pop_count = 0
			# Choose between print or write
			if inp == ":w":
				NodeClass.file_ptr_text = open("Compressed_text.txt", "wb")
				NodeClass.file_ptr_indx = open("Compressed_indx.txt", "wb")
				NodeClass.print_or_write = NodeClass.write_details
			else:
				NodeClass.print_or_write = NodeClass.print_details
			# Traverse node
			last_node.traverseNode()
			# close file if open
			if inp == ":w":
				NodeClass.file_ptr_text.close()
				NodeClass.file_ptr_indx.close()
			# Say some stats
			print(f"Ratio 16-bit deltas: {NodeClass.temp1/(NodeClass.temp1 + NodeClass.temp2 + NodeClass.temp3)}")
			print(f"Ratio 24-bit deltas: {NodeClass.temp2/(NodeClass.temp1 + NodeClass.temp2 + NodeClass.temp3)}")
			print(f"Ratio 32-bit deltas: {NodeClass.temp3/(NodeClass.temp1 + NodeClass.temp2 + NodeClass.temp3)}")
	else:
		last_node = motherNode.searchElement(inp)
	
	


