In part 3, we were supposed to explore the relationship between throughput and latency, and whether it conforms to the predictions of queuing theory.

We use the MLC tool's --loaded_latency option to calculate bandwidths at different latencies.

The MLC controls queue length (and hence latency) by inserting delays so as to throttle the memory-servers utilization. This results in different throughput at different latencies.

The command used was:

./mlc --loaded_latency -e -r

where,
-e tells MLC not to change prefetcher's settings
-r accounts for random memory access patern

The results was:

Delay   (ns)    MB/sec   
==========================
 00000  228.79    16197.2
 00002  227.69    16213.2
 00008  217.41    16201.4
 00015  214.18    16126.7
 00050  187.64    16094.2
 00100  117.77    15124.6
 00200   83.86    10160.3
 00300   77.95     7437.1
 00400   74.61     5947.3
 00500   74.61     5033.3
 00700   71.14     3932.7
 01000   71.25     3047.6
 01300   77.37     2502.7
 01700   80.97     2082.8
 02500   81.34     1670.8
 03500   81.42     1419.2
 05000   81.18     1233.3
 09000   79.64     1052.4
 20000   79.59      916.4

 We plotted this information after normalizing the bandwidth to its maximum, which essentially equals to server utilization (µ) from queueing theory* 

 The graph is given below

 ![plot](./PointerBufferSize_vs_Latency.PNG) 

 The graph describes latency as a function of normalized-throughput. We also plotted the theoretical latency along with measured latency. The theoretical latency can be calculated as follows from queuing theory:

 $Lq = (µ^2)/(1-µ)$
 $Tq = Lq*Ts + µ*Ts$
 $Tq = (µ^2)/(1-µ)*Ts + µ*Ts$
 $Tq = Ts*(µ^2 + µ(1-µ))/(1-µ)$
 $Tq = Ts*(µ/(1-µ))$
 $Latency = Tq+Ts = Ts*(µ/(1-µ)) + Ts$

 We know $Ts$ to be the server latency, it is the latency when queue-length is 0. We calculated it in part 1. It was around 62 ns.


 *We know that,

 $µ = λ.Ts$
 $λ = Arrival Rate = Departure Rate = Throughput$
 $Ts = Avg. Service Time or Server Latency$
 $1/Ts = B = Average Service Rate (when µ = 1); we know this from part 2 when we extracted max bandwidth$

 So,
 $µ = λ/B = Throughput/Max Bandwidth = Server Utilization = Fraction of bandwidth consumed$