//g++ -no-pie -o test2.out test2.cpp -ltut -Wl,-z,now

//	the now indincates the DT_BIND_NOW will be present

// using a shared library but without! lazy binding

#include <iostream>

//	foo is from shared library call libtut.so

void foo(int num);

int main(){
	foo(2);
	return 0;
}
