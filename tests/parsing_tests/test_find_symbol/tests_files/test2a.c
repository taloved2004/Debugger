//g++ test2a.cpp test2b.cpp -o test2.out -no-pie

#include <stdio.h>

void foo(int a);

int main(){
	foo(2);
	return 0;
}
