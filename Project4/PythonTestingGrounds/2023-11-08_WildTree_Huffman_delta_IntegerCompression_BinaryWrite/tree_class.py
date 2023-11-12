#! /usr/bin/python3
import array

class NodeClass:
	node_count = 0
	index_count = 0
	distinct_count = 0
	pop_count = 0
	push_count = 0
	temp1 = 0
	temp2 = 0
	temp3 = 0

	scan_stack = []

	file_ptr = None
	print_or_write = None

	def __init__(self):
		self.subnode = [None] * 27		# 0 to 26
		self.subnode[0] = []			# The 0th element has occurrence index count
		NodeClass.node_count += 1

	def addElement(self, word):
		# if the word has been encoded add in this sequence, add the index tag now.
		if len(word) == 0:
			NodeClass.index_count += 1
			# Keep track of distincts
			if len(self.subnode[0]) == 0:
				NodeClass.distinct_count += 1

			self.subnode[0].append(NodeClass.index_count)
			return

		# If there is still some of the word to be enqueued ...
		key = ord(word[0]) & 0x1F
		
		# if such a node does not exist, make one
		if self.subnode[key] is None:
			self.subnode[key] = NodeClass()

		# Keep enqueuing the word
		self.subnode[key].addElement(word[1:])

	# returns the node that could be the word or prefix.
	def lookup(self, word):
		# if the word is found at this node, return this node
		if len(word) == 0:
			return self
		
		# If not yet found ...
		key = ord(word[0]) & 0x1F

		# if such a subnode does not exist, return
		if self.subnode[key] == None:
			return None

		# if such a subnode exists, but is not terminal, look deeper still
		return self.subnode[key].lookup(word[1:])

	def searchElement(self, word):
		returned_node = self.lookup(word)

		if returned_node is None:
			print("Word not found")
		else:
			if len(returned_node.subnode[0]) != 0:
				print("Word found at indices:")
				# iterate over indices
				for idx in returned_node.subnode[0]:
					print(idx, end=", ")
				# print new line
				print()
			else:
				print(f"{word} is a prefix.")

		return returned_node

	def traverseNode(self):
		if len(self.subnode[0]) > 0:
			NodeClass.print_or_write(self.subnode[0])
			NodeClass.pop_count = 0
			NodeClass.push_count = 0
		
		# go deeper
		for idx in range(1, 27):
			if self.subnode[idx] != None:
				NodeClass.scan_stack.append(chr(96+idx))
				NodeClass.push_count += 1
				self.subnode[idx].traverseNode()
				NodeClass.scan_stack.pop()
				NodeClass.pop_count += 1

		# return void
		return

	####################### PRINTING SUBROUTINES #########################
	def print_details(idx_list):
		print("<", NodeClass.pop_count, ">", sep="")
		# Print this one
		for character in NodeClass.scan_stack:
			print(character, end="")
		
		print(": ", end="")

		for idx in idx_list:
			print(idx, end=", ")

		print()

	def write_details(idx_list):
		string_to_write  = "<" + str(NodeClass.pop_count)
		string_to_write += "".join(str(x) for x in NodeClass.scan_stack[len(NodeClass.scan_stack) - NodeClass.push_count:])
		string_to_write += "|"
		NodeClass.file_ptr.write(string_to_write.encode())

		# make a delta compressed idx_list
		compressed_idx_list = []

		for idx in range(1, len(idx_list)):
			compressed_idx_list.append(idx_list[idx] - idx_list[idx-1])

		# Write a short to show the number of elements
		data  = array.array("H", [len(idx_list)]).tobytes()

		# Write an int to show the original offset
		data += array.array("I", [idx_list[0]]).tobytes()

		# Write an array of shorts for deltas
		if len(compressed_idx_list) > 0:
			for elem in compressed_idx_list:
				if elem <= 2**14-1:					# Can put in 14 bits? [2'b10, 14'hXXXX]
					NodeClass.temp1 += 1
					data += array.array("H", [elem | 0x8000]).tobytes()	# elem is already < 2**14-1 so no need for & 0x3FFF
				elif elem <= 2**23-1:			# Can put in 23 bits? [1'b0, 23'hXXXXXX]
					NodeClass.temp2 += 1
					data += array.array("B", [elem >> 16]).tobytes()
					data += array.array("H", [elem & 0xFFFF]).tobytes()
				else:								# Fit in 30 bits. [2'b11, 30'hXXXXXXXX]
					NodeClass.temp3 += 1
					data += array.array("I", [elem | 0xC0000000]).tobytes()

		NodeClass.file_ptr.write(data)

		return

		

"""		USAGE RUBRIK

motherNode = NodeClass()

motherNode.addElement('asad')
motherNode.addElement('naveed')
motherNode.addElement('asad')
motherNode.addElement('naveeda')
motherNode.addElement('naveed')
motherNode.addElement('asad')

print("Node Count: ", NodeClass.node_count)

print("\n\nLOOK UP ...")
motherNode.searchElement('asad')
"""
