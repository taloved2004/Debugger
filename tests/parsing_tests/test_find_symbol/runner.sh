#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

EXIT_STATUS=0

echo "Compiling files..."
echo
echo ------------------
echo

#compile main test file
g++ --std=c++11 -o test_find_symbol.out main.cpp

#creare shared library
gcc -shared -fPIC -o libtut.so tests_files/libtut.c
sudo mv libtut.so /usr/lib/

#compile executable of test files
gcc tests_files/test1.c -no-pie -o tests_files/test1.out
gcc tests_files/test2a.c tests_files/test2b.c -no-pie -o tests_files/test2.out
gcc tests_files/test3.c -no-pie -o tests_files/test3.out
gcc tests_files/test4.c -no-pie -o tests_files/test4.out -ltut -Wl,-zlazy
gcc tests_files/test5.c -no-pie -o tests_files/test5.out -ltut -Wl,-z,now
gcc tests_files/test6.c -o tests_files/test6.out -no-pie
gcc tests_files/test7a.c tests_files/test7b.c -o tests_files/test7.out -no-pie
gcc tests_files/test8a.c tests_files/test8b.c -o tests_files/test8.out -no-pie


#run tests and save outputs
for (( i=1; i<=8; i++ ));
do
        ./test_find_symbol.out foo tests_files/test${i}.out > outputs/out${i}.txt
done


echo "Running diff"
echo
    for (( i=1; i<=8; i++)); do
        diff "./exp_outputs/exp${i}.txt" "./outputs/out${i}.txt" # &> /dev/null
        if [ $? -eq 0 ]; then
            echo -e "test ${i}: ${GREEN}PASS${NC}"
        else
            EXIT_STATUS=1
            echo -e "test ${i}: ${RED}FAIL${NC}"
        fi
    done

echo




#remove executable files
for (( i=1; i<=8; i++ ));
do
        rm tests_files/test${i}.out
done


rm ./test_find_symbol.out
