#!/bin/bash
#print messege
echo "Compiling files..."
echo

#create debugger
g++ --std=c++11 -o debug  ./Files/*.cpp

#check if it's for debugging - if so create minor tests
if [ -z "$1" ]
then
	exit 0
fi

echo "Compiling minor tests files..."

#call compile script from minor tests folder

chmod +x ./tests/minor_tests/compile.sh
cd ./tests/minor_tests
./compile.sh
cd - > /dev/null
