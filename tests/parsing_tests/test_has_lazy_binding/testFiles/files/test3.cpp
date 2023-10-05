//g++ -no-pie -o test3.out test3.cpp -ltut -Wl,-zlazy

// using a shared library with lazy binding

#include <iostream>

//	foo is from shared library call libtut.so

void foo(int num);

int main(){
	foo(2);
	return 0;
}
