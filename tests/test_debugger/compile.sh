#!/bin/bash

#compile static linking tests
for (( i=1; i<=4; i++));
do
	gcc test_static_linking/test${i}.c -no-pie -o test_static_linking/test${i}.out
done

#create a shared library
gcc -shared -fPIC -o libdyn.so test_dynamic_linking/libdyn.c
sudo mv libdyn.so /usr/lib/

#compile dynamic linking tests without lazy binding
for (( i=1; i<=5; i++));
do
        gcc test_dynamic_linking/test${i}.c -no-pie -o test_dynamic_linking/test${i} -ldyn -Wl,-z,now
done

#compile dynamic linking tests with! lazy binding
for (( i=1; i<=5; i++));
do
        gcc test_dynamic_linking/test${i}.c -no-pie -o test_dynamic_linking/test${i}_lazy -ldyn -Wl,-zlazy
done


