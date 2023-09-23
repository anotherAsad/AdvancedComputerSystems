The objective of project 1 is to gain deeper understanding on cache and memory hierarchy.

For parts 1 to 3, I use the Intel's MLC tool.

For parts 4 and 5, I have written codes which are qualified using Linux's perf command.

In order to accurately comment on the quantifiable parameters of memory system on a computer, one must know the cache/memory hierarchy of the computer.
The memory hierarchy of my laptop are as follow:

Cache ID 0:
- Level: 1
- Type: Data Cache
- Sets: 64
- System Coherency Line Size: 64 bytes
- Physical Line partitions: 1
- Ways of associativity: 8
- Total Size: 32768 bytes (32 kb)
- Is fully associative: false
- Is Self Initializing: true

Cache ID 1:
- Level: 1
- Type: Instruction Cache
- Sets: 64
- System Coherency Line Size: 64 bytes
- Physical Line partitions: 1
- Ways of associativity: 8
- Total Size: 32768 bytes (32 kb)
- Is fully associative: false
- Is Self Initializing: true

Cache ID 2:
- Level: 2
- Type: Unified Cache
- Sets: 1024
- System Coherency Line Size: 64 bytes
- Physical Line partitions: 1
- Ways of associativity: 4
- Total Size: 262144 bytes (256 kb)
- Is fully associative: false
- Is Self Initializing: true

Cache ID 3:
- Level: 3
- Type: Unified Cache
- Sets: 8192
- System Coherency Line Size: 64 bytes
- Physical Line partitions: 1
- Ways of associativity: 12
- Total Size: 6291456 bytes (6144 kb)
- Is fully associative: false
- Is Self Initializing: true

RAM:
- 8 Gigabytes SO-DIMM