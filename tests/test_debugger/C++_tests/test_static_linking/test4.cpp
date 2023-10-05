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
//bar - 4
//foo - 3
//bar - 2
//foo - 1 
//exit

int main(){

int b;
b = bar(4);
return 0;
}
