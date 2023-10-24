<h1>Project 3</h1>

Project 2 is intended to demonstrate the impact of memory access paterns, instruction-level parallelism and thread-level parallelism on task execution. In this project, we demonstrate various methods to accelerate matrix-matrix multiplication, compare the results, and comment on how these results can be explained.


<h2>1. Experimental Setup</h2>

keywords: `fio jobs`, `python scripts`, `read-write load simulation`, `JSON output`, `Queue length equivalence`

<h4>Hardware Used:</h4>
256 GB NVMe SSD. 512 MB seperate drive created for all fio experiments.

<h4>Command Format:</h4>
An example command format which uses the job description file `jobfile.fio` is given below:<br>

```bash
sudo fio --output=../output_files/outfile.json --output-format=json --bs=4k --iodepth=8 jobfile.fio
```

<h4>Experiment Overview and Organization:</h4>

- How experiment is organized
- Why so

The code is organized into multiple files for modularity. Since we have to use various types of matrix multiplications, we use function pointers as a means of templating, this allows us to dynamically change the function to be called on the basis of matrix multiplication and optimization required. This allows for code reusability, and a common execution interface for various optimization/data types.

The common arguments to various parts of code are passed through global structs like `execution_mode` and `mat_info`.
<br>
The execution flow of the program is shown below (Matrix initialization not shown)

In order to efficiently utilize `AVX 2`, the we use `mmap` instead of `malloc` to allocate multiple pages to our program. Memory mapped through `mmap` is always page-aligned. This has the added advantage of guarding against unaligned accesses (accesses on the edge of cache-line or `AVX` read granularity), which should give us better performance.

All the matrices are dynamically allocated and initialized. This means that the 2-D arrays are manually organized. We exploit this excess of control by saving matrix rows (or sometimes, columns) in a contiguous, 1-D fashion. This helps with drastically increasing **spatial locality** and **pre-fetching** potential.

The program outputs all results in a file in **CSV format**. This allows for ease of visualization and result data analyses. 
<br>
The program is compiled using the following command:
<br>

`gcc short_mat_init.c float_mat_init.c short_mat_funcs.c float_mat_funcs.c tiling_agents.c main.c -mavx2 -o a.out`

<h3>Note on Multi-Job Mode and single job equivalence</h3>
It seems that the total queue length = queue length of a single job * number of jobs

<h2>Optimizations Used</h2>

keywords: `NVMe support for multiple queues`, `parallel jobs`, `python script`, `JSON processing`

<h2>Results & Analyses</h2>

<h3> R/W Ratio of 100:0</h3>

- script output screenshots
- Comments

<h3> R/W Ratio of 50:50</h3>

- script output screenshots
- Comments

<h3> R/W Ratio of 75:25</h3>

- script output screenshots
- Comments

<h3> R/W Ratio of 0:100</h3>

- script output screenshots
- Comments

Prelim conclusion and analysis

Increasing queue length increases server utilization $µ$, which makes higher bandwidth possible

1. Graph for one case: (queue length and BW on x-axis, Latency on y-axis)

2. Graph for one case: (IOPS, Blocksize and Bandwidth)

_Table of execution time of various optimization texhniques under different matrix sizes_:
![graph](./float_table.PNG)

_Comparison of execution time in seconds for various optimization techniques_:
![graph](./float_results.png)

<h2>Conclusion</h2>

keywords: `NVMe device`, `comparison with intel (targets very low latency)`, `BW/lat relation seen`, `When to use IOPS`

- When optimizing a task, we must first identify the bottle-neck in the execution. Failing to resolve the bottle-neck may cause other unrelated optimizations to not bear any effect in speeding up the execution.
- Conversely, resolving bottle-necks in order of their significance can give us iteratively better implementations.
- Moreover, multiple optimizations targetting same bottleneck may not result in better performance, because the bottleneck might have shifted elsewhere (e.g. from memory to compute).
- When parallelizing an implementation, one must be careful about the atomicity of access. If the parallel parts of the program share data, mutexes and semaphores must be used to avoid race conditions.
- In order to reap the benefits of instruction-level parallelism, it is important that the application not be memory-bound. Therefore, the program should be structured to keep the processor occupied by providing high memory-access throughput.
