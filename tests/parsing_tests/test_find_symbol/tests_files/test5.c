//g++ test5.cpp -o test5.out -ltut -wl,z,now -no-pie

#include <stdio.h>

//should not find the symbol
void foo(int a);

int main(){
	foo(2);
	return 0;
}
