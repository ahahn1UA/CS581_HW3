#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load intel
module load gcc/11.3.0

./HW3.out 5000 5000 20 20thread_isx1
./HW3.out 5000 5000 20 20thread_isx2
./HW3.out 5000 5000 20 20thread_isx3
./HW3_gcc.out 5000 5000 20 20thread_gcc1
./HW3_gcc.out 5000 5000 20 20thread_gcc2
./HW3_gcc.out 5000 5000 20 20thread_gcc3