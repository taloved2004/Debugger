//gcc tests_files/test6.c -o tests_files/test6.out -no-pie

#include <stdio.h>

//should not find the symbol
int foo = 1;

int main(){
	int a = foo + foo;
	return 0;
}
