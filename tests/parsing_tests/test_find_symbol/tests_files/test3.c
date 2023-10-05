//g++ test3.cpp -o test3.out -no-pie

#include <stdio.h>

//	should not find the function
static void foo(int a){
	a++;
}

int main(){
	foo(2);
	return 0;
}
