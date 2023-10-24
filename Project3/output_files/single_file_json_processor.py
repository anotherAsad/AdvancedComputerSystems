#! /usr/bin/python3

import json
    
with open("outfile_blk4k_qlen1_rw0_100.json", "r") as read_file:
	data = json.load(read_file)


bw_kb  = 0
iops   = 0
lat_ns = 0 

for job in data["jobs"]:							# iterates over all (4) jobs
	job_type = job["job options"]["rw"]
	job_type = job_type[4:]
	
	bw_kb += job[job_type]["bw"]						# bw in kbytes
	iops  += job[job_type]["iops"]
	lat_ns += job[job_type]["lat_ns"]["mean"]
	
	
print(f"bw_kb: {bw_kb}, iops: {iops}, lat_ns: {lat_ns/4}")


