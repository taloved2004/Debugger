//g++ test1.cpp -o test1.out -no-pie
//gcc test1.c -no-pie -o test1.out
#include <stdio.h>

void foo(int a){
	a++;
}

//void bar(int a){
	//a++;
//}

int main(){
	foo(2);
	return 0;
}
