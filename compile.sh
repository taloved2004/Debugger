#!/bin/bash

#create debugger
g++ --std=c++11 -o debug  ./Files/*.cpp

#create shared library for tests purposes
gcc -shared -fPIC -o libdyn.so ./tests/test_debugger/test_dynamic_linking/libdyn.c
sudo mv libdyn.so /usr/lib/

#create minor tests
gcc -o ./tests/minor_tests/test.out ./tests/minor_tests/test.c -no-pie
gcc -o ./tests/minor_tests/dyn_test.out ./tests/minor_tests/dyn_test.c -no-pie -ldyn -Wl,-z,now
gcc -o ./tests/minor_tests/lazy_dyn.out ./tests/minor_tests/dyn_test.c -no-pie -ldyn -Wl,-zlazy
gcc -o ./tests/minor_tests/pic_test.out ./tests/minor_tests/test.c 
