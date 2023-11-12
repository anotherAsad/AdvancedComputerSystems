#! /usr/bin/python3

class NodeClass:
	curr_idx = 0
	node_count = 0

	def __init__(self):
		self.subnode = [None] * 27		# 0 to 26
		self.subnode[0] = []				# The 0th element marks
		NodeClass.node_count += 1

	def addElement(self, word):
		# if the word has been encoded add in this sequence, add the index tag now.
		if len(word) == 0:
			NodeClass.curr_idx += 1
			self.subnode[0].append(NodeClass.curr_idx)

			print("This word is now on indices:")

			for x in self.subnode[0]:
				print(x)

			return

		# If there is still some of the word to be enqueued ...
		key = ord(word[0]) & 0x1F
		
		# if such a node does not exist, make one
		if self.subnode[key] is None:
			self.subnode[key] = NodeClass()

		# Keep enquing the word
		self.subnode[key].addElement(word[1:])


	def lookup(self, word):
		# if the word is found at this node
		if len(word) == 0:
			print("This word is at indices: ", )

			for x in self.subnode[0]:
				print(x)
			
			return True
		
		# If not yet found ...
		key = ord(word[0]) & 0x1F

		# if such a subnode does not exist, return
		if self.subnode[key] == None:
			return False

		# if such a subnode exists, but is not terminal, look deeper still
		return self.subnode[key].lookup(word[1:])

	def searchElement(self, word):
		if not self.lookup(word):
			print("Word not found")
		# return void
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
