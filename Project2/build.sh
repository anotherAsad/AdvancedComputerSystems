#! /bin/bash

gcc short_mat_init.c float_mat_init.c short_mat_funcs.c float_mat_funcs.c tiling_agents.c main.c -O7 -mavx -mavx2 -o a.out