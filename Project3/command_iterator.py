#! /usr/bin/python3
import os

for bsize in [4, 16, 32, 128, 256]:									# block_size_in_KiB
	for qlen in [1, 8, 32, 256, 1024]:								# single target_queue_len
		for rw in ["100_0", "0_100", "50_50", "70_30"]:				# rw_ratios
			rd_percent = int(rw.split("_")[0])													# percentage of reads
			command = f"sudo fio --output-format=json --output=out_dir/outfile_blk{bsize}k_qlen{qlen}_rw{rw}.json --bs={bsize}k --iodepth={qlen} --rwmixread={rd_percent} jobfile.fio"
			print(command)
			print("Executing...")
			os.system(command)
			print()
