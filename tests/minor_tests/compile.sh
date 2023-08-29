#!/bin/bash

#create minor tests
gcc -o test.out ./test.c -no-pie
gcc -o dyn_test.out ./dyn_test.c -no-pie -ldyn -Wl,-z,now
gcc -o lazy_dyn.out ./dyn_test.c -no-pie -ldyn -Wl,-zlazy
gcc -o pic_test.out ./test.c 
