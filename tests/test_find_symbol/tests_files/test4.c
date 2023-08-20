//g++ test4.cpp -o test4.out -Wl,-zlazy -no-pie

#include <stdio.h>

//should not find the symbol
void foo(int a);

int main(){
	foo(2);
	return 0;
}
