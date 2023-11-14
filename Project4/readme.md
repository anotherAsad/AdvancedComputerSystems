<h1>Project 4</h1>

Project 4 is the implementation of dictionary codec, that enables operations like compression and fast look-up/scan on a column of data with low cardinality.<br>

We implement an N-ary tree to aid encoding and search/scan operations on the given column. We also implement integer compression techniques like delta and Huffman encoding to reduce SSD footprint.<br>

Due to some optimizations considering low-cardinality columns, our on-disk size of the compressed file is`405 MB`, which beats the **WinRAR's best compression option** of `475 MB` by ~15%. It also boasts two methods for search/scan, one for extremely low lookup latency, and the other for small main memory footprint.

The project also makes use of multi-threading and SIMD to speed up encoding and lookup operations respectively.

<h2>Design Strategies & Techniques Used</h2>

keywords: `N-ary tree`, `prefix`, `prototyping and analysis`, `Delta encoding`, `Huffman coding`


Considering that our target is to compress a low-cardinality column, with lots of elements that are either repeated or share a common prefix, a **tree-like** structure seems a natural choice. As opposed to hashes, trees have the added advantage of eleminating repetitions due to common prefix, and enabling natural prefix scans.<br>

For simplicity, we have used a data-structure that extends an N-ary tree. This datastructure can be used for encoding, element search, prefix scan, and can be modularly extended to enable writing of the compressed column to disk, or a separate data structure.<br>

<h3>Data Representation in the N-ary Tree</h3>

In our data structure, every word is stored character-wise in the tree nodes. Every node in the tree stores a character, and has a set of pointers pointing to the next character of the word.<br>
If the node is terminal (i.e., represents the last character in a string), it also stores a dynamic list of indices where the string appears in the uncompressed column. This way, repeating words are only represented once (with their different indices stored in a leaf node representing the last character). Words with common prefixes diverge at the first different character, and hence are also represented with as little repetition as possible.

To illustrate, words "carmen" and "carpet" would be represented with 9 nodes in total, with the common prefix "car" saved only once. The node representing "r" would have two pointers - to "m" and "p" - for the non-simmilar parts of the words.

This type of representation is especially amicable to the operation of prefix search, as will be seen later.

<h3>Integer Compression</h3>

For on-disk storage, we combined two integer compression techniques:

1. **Delta compression**: Except for the first element, each element $c_i$ in the compressed integer sequence is expressed as the difference of the elements $a_i$ and $a_{i-1}$ in the original list. The first element of both the compressed and the original list is the same. Delta compression works well when the jumps between integers of a sequence are smaller than their absolute values, which could further aid the quality of compression by huffman coding.

2. **Huffman coding**: The original sequence stores integers in 32-bit unsigned format. With Huffman encoding enabled, we use variable sizes of integers to store the sequence elements. The MSbits of every integer in the compressed sequence correspond to its size: if the MSbit is `0`, it's a 23-bit integer stored in 3 bytes; if the MSbit is `1` then it's a 14-bit integer stored in 2 bytes (if the 2nd MSbit is `0`), or a 30-bit integer stores in 4 bytes (if 2nd MSbit is `1`).<br> Huffman coding adds some complexity to the decoding process, but this alone results in a **25 %** reduction in the size of the integer sequence.

<h3>String Compression</h3>
Apart from integer compression, we also use a variant of delta coding for string compression of the on-disk compressed file. Here, every word is represented in terms of its difference from the previous word: only the different part of the later word is store. A single character denoting the length of the difference is also stored in this technique.

To illustrate, let's say we save "carmen" and "carpet". In storage, they would look like: "carmen<3>pet", where <3> denotes that the last three characters of "carmen" have to be discarded, and the string "pet" has to be added to get the new word "carpet". This techniques is quite useful when we store the data in alphabetical order (which is naturally done if we traverse our tree in depth first order).<br>

To test the feasibility of our design decisions, a prototype was first implemented in `python`. The analysis of different compression techniques used is given in the following section.

<h2>Analysis of compression techniques</h2>
keywords: `Integer compression`, `string compression`, `delta coding for strings`, `on-disk compressed storage`

A preliminary analysis of the compression techniques for the given column file was performed. The results, aided by an implementation of the tree structure in `python`, are given below.

<h4>Column Details:</h4>

```
Total number of elements: 139999654 (~133.5 Mega Entries)
Unique elements: 975773 (~0.93 Mega Entries)
Unique-to-Total ratio: 0.7 % or ~1/140
Average entry size: 8 characters

Size before compression:
	1,120,173,090 bytes (~1,069 MB)
Best Theoretical size post compression (Repetition elimination only, assuming 4-byte ints):
	975773*8 + 139999654*4 = 567,804,800 bytes (~541 MB), i.e., 50% of the original.
```

<h4>Integer Compression details</h4>

We start with an uncompressed sequence of integers that represent indices of column elements. Originally, each integer occupies 4 bytes. After performing `Huffman & Delta encoding`, the integer sizes in the compressed sequence have following ratios:

```
Post-compression size ratios:
16-bit Ints: ~ 2.4%
24-bit Ints: ~ 97.3%
32-bit Ints: ~ 0.22%
```

This corresponds to an average of 2.975 bytes per integer in compressed stream, which should compress the all the integers by around 25%.

Experimentally we see:
```
Size of all indices before compression = 139999654 * 4 = 5599998616 Bytes = ~ 534 MB. 
Post-Huffman & delta encoding compressed size (Indices only): 419,808,734 bytes = 400.36 MB.
```

The post compression file size matches expectations, since 534 MB *3/4 = 400.5 MB.


<h4>Text Compression Details</h4>

```
Size estimate of unique text : (975773 uniques * 8 B average): ~ 7,806,184 bytes = ~ 7.44 MB
Size after string delta encoding: 6,267,526 bytes = ~5.97 MB
Delta Encoding Advantage: 6,267,526/7,806,184 = 80.2%
```

<h4>Total Post Compression Size</h4>

Post-Huffman & delta encoding compressed size (Full File): 426,076,260 bytes = ~406 MB

Which is $406 MB/1069 MB$ i.e. only $38\%$ of the original file size.

<h3>Conclusion</h3>

As discussed, eliminating repetitions accounts for around 50% reduction. Compounding over that, integer compression can reduce the size by an additional **25%**. However, string delta compression only saves us around **1.5 MB**, which amounts to almost nothing in the grand scheme of things. So the in-memory size is around **534 MB**, and on-disk size is **406 MB**.

<h2>Code Overview</h2>
keywords: `tree node`, `fast lookup`, `multi-threading for encoding`, `SIMD support for lookup`

The code is primarily centered around a C++ class called `treenode`, which represents a single tree node. The class has methods like `addElement` and `lookup`.

The `addElement` method simply adds an element to the node, making a new node sequence if the element has not appeared before, or coalescing it with prior nodes if the element has appeared before. It naturally forks-off at the apropriate point if the element being added is unique, but shares a prefix (of any length) with prior elements.

The `lookup` method searches the tree for a given sequence of characters, and returns a node that represents the last character of the sequence. This returned node can then be used for both **prefix scan** and **single element search**. For **single element search**, we simply 

<h2>1. Experimental Setup</h2>

keywords: `fio job description file`, `python scripts`, `JSON output`

<h4>SSD under test:</h4>
Toshiba KXG60ZNV256G - 256 GB NVMe SSD.<br>
512 MB seperate drive created for all fio experiments.

<h4>fio job description file:</h4>

The **fio** command-line utility can take a job description file as input. The job description file contains the params for the IO job to be carried out. Contents of the job description file used in this project are given below, We use this file to pass common parameters for all out tests:

```
; -- start job file --
[Job1]
ioengine=libaio             ; use the standard linux async IO library
;iodepth=32                 ; Target queue depth to be maintained. Impacts bandwidth. 
;bs=4k                      ; Block size is the unit of IO to be read or written, i.e., the data access size.
numjobs=1                   ; Number of identical processes to be spawned for each job.  
direct=1                    ; Avoid kernel caching for disk IOs.
fsync=0                     ; Do not issue fsync after every access. Setting it to 1 drastically diminishes IOPS and BW.
filename=/dev/nvme0n1p5     ; Name of the target drive for test
rw=randrw	                ; Type of access pattern. Can be randwrite, randread, randrw and all their sequential counterparts
size=512m                   ; Size of the test reqion. Number of IOs = size/bs.
; -- end job file --
```

<h4>Command Format:</h4>

A bash command that invokes the fio utility is given below. All flags passed are used for parameters that change across tests.

```bash
sudo fio --output-format=json --output=../out_dir/outfile.json --bs=4k --iodepth=8 --rwmixread=50 jobfile.fio

# Flag Description:
# --output-format=json  |   Save output in JSON format. As opposed to simle text, JSON format can be easily parsed at result collection stage.
# --bs=4k               |   Data access size for a single IO is 4K. Overrides the option in jobfile.fio
# --iodepth=8           |   Target queue depth is 8
# --rwmixread=50        |   Percentage of reads in total IOs
# jobfile.fio           |   Job description file used.
```

<h3>Stats Collection Overview</h3>

The assignment calls for measuring the stats for various combinations of workloads. In order to efficiently collect these stats we use a few python scripts:

1. We use `command_iterator.py` to iterate of **data access sizes**, **target queue lengths** and **read/write ratios**, and issue fio commands that write their results to the `out_dir` directory. For ease of parsing, the outputs are in `JSON` format.
2. We use `json_processor.py` to parse the results and extract **bandwidth**, **IOPS** and **latency** stats for different workload combinations. The extracted results are written to a **CSV file** for tabulation and graphing of results.

<h2>Results & Analyses</h2>

This section summarizes with the results of the **fio** experiments extracted via `json_processor.py`. The results are categorized according to read-write ratios.

<h3> Case I: Results for R/W Ratio of 100:0</h3>

_Table of statistics for read-vs-write ratio of 100:0_
![graph](./Table_RW_100_0.PNG)

- The relationship between IOPS and throughput (called bandwidth in context of **fio**) agrees with the theory, i.e., $Bandwidth = IOPS * SizeOfDataAccess$.
- [*] Increase in queue length corresponds with increase in bandwidth (and also IOPS) along with latency. This is in line with the queuing theory: Higher queue length means better server utilization, which allows the queue server to achieve a higher fraction of maximum bandwidth. However, it also increases latency due to queue width $T_q$.
- Larger data access sizes result in higher bandwidth, however the increase in bandwidth is usually not proportional to the increase in data access size. This is because larger data access sizes put a strain on the interconnect, as there is more data to be moved around. So in this case, the interconnect bandwidth at the junction of the NVMe interface, and the device, becomes the bottleneck.<br> To illustrate, if we have infinite interconnect or external bandwidth, doubling data access size should double bandwidth because the IOPS would remain the same (up until the access size is less than NAND page size). But in the case of limited external bandwidth, IOPS actually go down with increase in data access size, which results in a less-than-proportional increase in the bandwidth.

<h3> Case II: Results for R/W Ratio of 0:100</h3>

_Table of statistics for read-vs-write ratio of 0:100_
![graph](./Table_RW_0_100.PNG)

- As opposed to the case of 100% reads, the case of 100% writes is surprisingly faster for lower queue sizes and lower data access sizes. This seemed an odd result at first, but I have verified it against [online benchmarks](https://ssd.userbenchmark.com/SpeedTest/358656/KXG50ZNV256G-NVMe-TOSHIBA-256GB).<br>There is a good explanation for this behavior, though. We know that, at the level of a NAND flash die, the write operation is very expensive: all writes must erase a whole block and re-write it again. However, to cirvumvent this issue, SSDs usually have large write buffers, which accumulate writes before flushing. And since a write is only uni-directional from the perspective of the rest of the system, write-bandwidth can actually appear faster than read-bandwidth for small access sizes. For larger access sizes, the read-bandwidth starts to win again, since the afore-mentioned write buffer gets filled more often.
- All the other observations are along the lines of Case I.

<h3> Case III: Results for R/W Ratio of 50:50</h3>

_Table of statistics for read-vs-write ratio of 50:50_
![graph](./Table_RW_50_50.PNG)

- The case of 50% reads and 50% writes performs noticeably than both the 100% cases above.
- All other observations from Case I still apply.

<h3> Case IV: Results for R/W Ratio of 70:30</h3>

_Table of statistics for read-vs-write ratio of 70:30_
![graph](./Table_RW_70_30.PNG)

- This case is very similar to Case III, especially for deeper queue sizes.
- All other observations from Case I still apply.

<h2>Observations</h2>

From the above experiments and there results, we can make a few salient observations:

1. Increasing queue length increases server utilization $µ$, which makes higher throughput possible. So the relationship between throughput and latency is also seen here. The two graphs below illustrate this.

_Bandwidth vs. Latency:_
![graph](./Bandwidth_vs_Latency.png)

_Queue Length vs. Latency:_
![graph](./Queue_Length_vs_Latency.png)


2. For smaller data access sizes, increasing queue length tends to increase IOPS (at least for some steps). But for larger data access sizes, IOPS do not change much with the queue length. This is explainable, since larger data access sizes tend to saturate the external bandwidth pretty quickly, limiting the IOPS to the external bandwidth.<br> This explains why IOPS are used by SSD vendors to demonstrate throughput for small data access sizes: It shows how fast the internal NAND architecture can be in the best of cases. However, since the external bandwidth becomes the limiting factor for bigger data access sizes, it is practical to report the maximum possible data transfer rate in MB/sec for large data access sizes.<br><br>This relationship is explored in the following graph which plots queue length against IOPS (for the case of 100% writes). As can be seen, IOPS stay constant across multiple queue lengths for the data access size of 128K, pointing to a transfer bandwidth saturation. 

![graph](./Queue_Length_vs_IOPS.png)

<h2>Conclusion</h2>
From this project, the trade-off between throughput and latency is reinforced again. We also see the significance of data-access size and queue-length in SSD performance, and the reasons why they could influence throughput. We also see that, although the individual physical read/write operations are very slow inside an SSD, the practice of parallelising elements inside an SSD allows for such high theoretical bandwidths that it may even be limited by the external bandwidth.

<h3>Why Intel D7-P5600 has 130k IOPS for random write-only 4KB case?</h3>

Since we could not find the exact queue depth for these tests, we assume QD32 or greater for comparison. In that case, our SSD demonstrates around 360K IOPS for random 4KB writes, that is, almost 3 times faster that P5600. Our SSD could beat the $7000 industrial grade SSD for 4KB random writes for the following reasons:

1. The P5600 rarely has to deal with data access sizes of 4KB, that is why it is not optimized for small data access sizes.
2. For higher reliability, the industrial SSD might use more aggressive error correction coding and fault-handling techniques. This may be the reason for low write IOPS.