For this part, we use the intel MLC tool's Idle Latency measuring feature to measure read latencies.

The Idle Latency feature ensures  zero-queue lengths by using dependent reads, i.e., the address of the (i+1)-th read is given by the value of i-th read.

The general structure of the MLC commands is:

mlc.exe --idle_latency -c0 -e -r -b[size_in_KiB]

where,
-c0 means using a single core (core 0)
-e tells the MLC tool to not change pre-fetcher settings
-r populates the buffer with random address values, this is done to beat the predictive potential of pre-fetcher.
-bn is the size of the buffer in KiB, changing the buffer size can also be used to target specific layers in memory hierarchy. For instance, -b256 initialized a contiguous buffer of 256 KiB, which matches the L2 cache size.

The results of these tests are shown in the figure below

![plot](./PointerBufferSize_vs_Latency.PNG)

As can be seen, the latencies are pretty low when targetting caches, but increase when we deal with the RAM. Also, once we cross the 24 MiB mark of Pointer Buffer size, the latencies stay constant. This is because after 24 MiB mark, the MLC tool predominantly targets the RAM, so latency remains the same regardless of buffer size.

The write latencies are almost the same as read latencies, considering the internals and architecture of RAM and caches.