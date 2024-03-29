In part 3, we were supposed to explore the relationship between throughput and latency, and whether it conforms to the predictions of queuing theory.

We use the MLC tool's --loaded_latency option to calculate bandwidths at different latencies.

The MLC controls queue length (and hence latency) by inserting delays so as to throttle the memory-servers utilization. This results in different throughput at different latencies.

The command used was:

./mlc --loaded_latency -e -r

where,
-e tells MLC not to change prefetcher's settings
-r accounts for random memory access patern

 We plotted the resulting information after normalizing the bandwidth to its maximum, which essentially equals to server utilization (µ) from queueing theory* 

 The graph is given below

 ![plot](./throughput_vs_latency.PNG) 

 The graph describes latency as a function of normalized-throughput. We also plotted the theoretical latency along with measured latency. The theoretical latency can be calculated as follows from queuing theory:

 $L_q = \(µ^2\)/\(1-µ\)$

 $T_q = L_q.Τ_s + µ.Τ_s$

 $T_q = \(µ^2\)/\(1-µ\).Τ_s + µ.Τ_s$

 $T_q = Τ_s.\(µ^2 + µ.\(1-µ\)\)/\(1-µ\)$

 $T_q = Τ_s.\(µ/\(1-µ\)\)$

 $Latency = T_q+Τ_s = Τ_s.\(µ/\(1-µ\)\) + Τ_s$

 We know $Τ_s$ to be the server latency, it is the latency when queue-length is 0. We calculated it in part 1. It was around 62 ns.

 It can be seen from the graph that the experimental results largely follow the predicitons of queuing theory.

 # Derivation of Max Bandwidth in Terms of $T_s$
 *We know that,

 $µ = λ.Τ_s$

 $λ = Arrival Rate = Departure Rate = Throughput$

 $Τ_s = Avg Service Time = Server Latency$

 $1/Τ_s = B = Avg Service Rate \(when µ = 1\)$; we know this from part 2 when we extracted max bandwidth

 So,

 $µ = λ/B =$ Throughput/Max Bandwidth $=$ Server Utilization $=$ Fraction of bandwidth consumed