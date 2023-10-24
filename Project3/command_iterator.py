#! /usr/bin/python3
import os

block_size_in_KiB = 4
iodepth = 4

#sample command
command = f"sudo fio --output=outfile.txt --bs={block_size_in_KiB}k --iodepth={iodepth} jobfile.fio"

for bsize in [4, 16, 32, 128]:											# block_size_in_KiB
	for qlen in [1, 2, 8, 64, 256]:									# single target_queue_len
		for rw in ["0_100", "75_25", "50_50", "100_0"]:					# rw_ratios
			command = f"sudo fio --output=output_files/outfile_blk{bsize}k_qlen{qlen*4}_rw{rw}.json --output-format=json --bs={bsize}k --iodepth={qlen} JobFiles/jobfile_RW_{rw}.fio"
			print(command)
			print("Executing...")
			os.system(command)
			print()
		
#os.system(command)
