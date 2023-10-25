#! /usr/bin/python3

import json

def extract_stats(job):
	[bw_kb, iops, lat_ns] = [0, 0, 0]

	for mode in ["read", "write"]:		
		bw_kb += job[mode]["bw"]								# bw in kbytes. Sum for both read and write
		iops  += job[mode]["iops"]								# IOPS
		lat_ns += job[mode]["lat_ns"]["mean"]					# Average latency
		
	# report average latency in ms
	lat_ns = lat_ns/2			
	lat_ms = lat_ns/1000

	return [bw_kb, iops, lat_ms]
	
		
for rw in ["100_0", "0_100", "50_50", "70_30"]:	#, "50_50", "75_25", "0_100"]:					# rw_ratios
	print(f"*** R/W ratio: {rw.replace('_', ':')} ***")
	print(f"========================")
	for bsize in [4, 16, 32, 128]:											# block_size_in_KiB
		for qlen in [1, 8, 32, 256, 1024]:									# total target_queue_len
			# Open file
			file =  open(f"out_dir/outfile_blk{bsize}k_qlen{qlen}_rw{rw}.json", "r")
			data = json.load(file)

			[bw_kb, iops, lat_ms] = extract_stats(data["jobs"][0])

			print(f"{bsize:3}k, {qlen:4}", end=", ")
			print(f"{bw_kb/1000:7.2f}, {iops/1000:6.2f}, {lat_ms:8.2f}")
			#print(f"Access Size: {bsize:3}k | Queue Depth: {qlen:4}", end="  ||  ")
			#print(f"BW: {bw_kb/1000:7.2f} MB/sec, iops: {iops/1000:6.2f} k, lat: {lat_ms:8.2f} ms")
		
	print("\n")
