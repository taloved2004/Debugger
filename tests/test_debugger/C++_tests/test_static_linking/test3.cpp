#include <iostream>


int bar(int m);
int foo( int n){
	if(n<=1)
		return 1;
	return bar(--n);
}

int bar(int m){
	if(m<=1)
		return 1;
	return foo(--m);
}

//foo - 3
//bar - 2
//foo - 1 
//exit

int main(){

	int b;
	b = foo(3);
	return 43;
}
