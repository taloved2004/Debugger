#!/bin/bash

#checks if no-pie folder exists - if not, creates it
static_no_pie_folder_name="test_static_linking/no-pie"

if [ ! -d "$static_no_pie_folder_name" ]; then
     mkdir "$static_no_pie_folder_name"
fi

dynamic_no_pie_folder_name="test_dynamic_linking/no-pie"
if [ ! -d "$dynamic_no_pie_folder_name" ]; then
    mkdir "$dynamic_no_pie_folder_name"
fi

static_pie_folder_name="test_static_linking/pie"

if [ ! -d "$static_pie_folder_name" ]; then
     mkdir "$static_pie_folder_name"
fi

dynamic_pie_folder_name="test_dynamic_linking/pie"
if [ ! -d "$dynamic_pie_folder_name" ]; then
    mkdir "$dynamic_pie_folder_name"
fi


#compile static linking tests
for (( i=1; i<=6; i++));
do
	gcc -static test_static_linking/test${i}.c -no-pie -o test_static_linking/no-pie/test${i}.out
	gcc -static test_static_linking/test${i}.c -o test_static_linking/pie/test${i}.out
done

#create a shared library
gcc -shared -fPIC -o libdyn.so test_dynamic_linking/libdyn.c
sudo mv libdyn.so /usr/lib/

#compile dynamic linking tests without lazy binding
for (( i=1; i<=5; i++));
do
        gcc test_dynamic_linking/test${i}.c -no-pie -o test_dynamic_linking/no-pie/test${i} -ldyn -Wl,-z,now
	gcc test_dynamic_linking/test${i}.c -o test_dynamic_linking/pie/test${i} -ldyn -Wl,-z,now
done

#compile dynamic linking tests with! lazy binding
for (( i=1; i<=5; i++));
do
        gcc test_dynamic_linking/test${i}.c -no-pie -o test_dynamic_linking/no-pie/test${i}_lazy -ldyn -Wl,-zlazy
	gcc test_dynamic_linking/test${i}.c -o test_dynamic_linking/pie/test${i}_lazy -ldyn -Wl,-zlazy
done


