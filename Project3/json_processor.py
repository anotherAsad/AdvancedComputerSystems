#! /usr/bin/python3

import json

def extract_stats(job_list):
	bw_kb  = 0
	iops   = 0
	lat_ns = 0 
		
	for job in job_list:							# iterates over all (4) jobs
		job_type = job["job options"]["rw"]
		job_type = job_type[4:]
		
		bw_kb += job[job_type]["bw"]						# bw in kbytes. Sum for all jobs
		iops  += job[job_type]["iops"]					
		lat_ns += job[job_type]["lat_ns"]["mean"]
		
	lat_ns = lat_ns/4				# Latency of all jobs should be averaged
	lat_ms = lat_ns/1000

	return [bw_kb, iops, lat_ms]
	
	
		
for rw in ["100_0", "50_50", "75_25", "0_100"]:					# rw_ratios
	print(f"*** R/W ratio: {rw.replace('_', ':')} ***")
	print(f"========================")
	for bsize in [4, 16, 32, 128]:											# block_size_in_KiB
		for qlen in [4, 8, 32, 256, 1024]:									# total target_queue_len
			# Open file
			file =  open(f"output_files/outfile_blk{bsize}k_qlen{qlen}_rw{rw}.json", "r")
			data = json.load(file)

			[bw_kb, iops, lat_ms] = extract_stats(data["jobs"])

			print(f"{bsize:3}k, {qlen:4}", end=", ")
			print(f"{bw_kb/1000:7.2f}, {iops/1000:6.2f}, {lat_ms:8.2f}")
			#print(f"Access Size: {bsize:3}k | Queue Depth: {qlen:4}", end="  ||  ")
			#print(f"BW: {bw_kb/1000:7.2f} MB/sec, iops: {iops/1000:6.2f} k, lat: {lat_ms:8.2f} ms")
		
	print("\n")


