#! /usr/bin/python3

import os

filestr = ""

global_params = f"""
; -- start job file --
[global]
ioengine=libaio
;iodepth=16
;bs=4k
numjobs=1
direct=1					; Avoid kernel caching.
fsync=0
filename=/dev/nvme0n1p5
"""

job_type = [0,0,0,0]					# 0 for read. 1 for write
job_mode = ["randread", "randwrite"]

filestr += global_params

for idx in range(0, 4):
	filestr += f"""
[j{idx+1}]
rw={job_mode[job_type[idx]]}
size=128m
"""

print(filestr)

f = open(f"""jobfile_RW_100_0.fio""", "w")
f.write(filestr)
f.close()
