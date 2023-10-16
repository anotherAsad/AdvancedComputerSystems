<h1>Project 2</h1>

Project 2 is intended to demonstrate the impact of memory access paterns, instruction-level parallelism and thread-level parallelism on task execution. In this project, we demonstrate various methods to accelerate matrix-matrix multiplication, and compare the results.

<h2>Code Organization</h2>

keywords: `function pointers`, `common execution interface`, `aligned memory mapping`, `dynamic allocation`, `CSV output`

The results are shown in figures below. The program prints the time taken for execution. Higher execution time for higher buffer sizes corresponds to greater cache miss ratio, because cache misses incur a cost of latency in accessing the lower levels and updating the cache contents. Moreover, the `perf` command reports the **unified L3 cache miss rate**, and the **L1-D cache miss rate** as well.
<br>

<h2>Optimizations Used</h2>

keywords: `Column Wise Matrix Storage`, `Matrix Tiling`, `Multi Threading`, `linearized 2-D array storage`, `SIMD`

<h2>Results</h2>


_Results for a buffer size of 32 KB. Targets L1-D cache primarily. In this case, the metric of concern is `L1-dcache-load-misses` at `perf` output_:

![graph](./short_table.PNG)

<img src="./short_table.PNG" width="800" height="500" />

_Results for a buffer size of 256 KB. Targets L2 cache primarily_:
<img src="./short_results.png" width="800" height="500" />

_Results for a buffer size of 6 MB. Targets L3 cache primarily. Note the increase in execution time_:
<img src="./float_table.PNG" width="800" height="500" />

_Results for a buffer size of 128 MB. Targets the off-chip RAM. The execution time increases significantly as off-chip accesses incur huge latencies_:
<img src="./float_results.png" width="800" height="500" />

The number of operations performed on these buffers of different sizes are exactly same, and are around 536 Million. We ensure this because we don't want any cache miss statistics reported that depend on factors other than memory heirarchy. One can observe this in the screenshots, as the number of instructions executed in all the cases is almost equal, but execution times are vastly different.


<h2>Conclusion</h2>

*The following graph shows the relationship between dominantly targeted memory type, and software performance. We can surmise that as we move further from the CPU towards off-chip memory, the execution time worsens.*
![graph](./Target_mem_vs_exec_time.PNG)

*This graph below shows the relationship between cache-miss rate and execution time. The miss rate is positively correlated with execution time, with the likely cause of increased memory access latency and cache-line update.*
![graph](./miss_rate_vs_exec_time.PNG)