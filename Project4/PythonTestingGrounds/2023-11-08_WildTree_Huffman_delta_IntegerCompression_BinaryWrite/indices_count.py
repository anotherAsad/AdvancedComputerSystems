#! /usr/bin/python3

file1 = open("Column_4Mentries.txt", "r")


dict1 = {}

for x in range(97, 97+26):
	dict1[chr(x)] = 0

dict1['\n'] = 0

for _ in range(0, 4*1024*1024):
	entry = next(file1)
	 
	for char in entry:
		dict1[char] += 1

for x in range(97, 97+26):
	print(chr(x), dict1[chr(x)])

print('\\n', dict1['\n'])