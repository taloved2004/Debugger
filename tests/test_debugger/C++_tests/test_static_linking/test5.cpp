#include <iostream>

int foo( int n){
	if(n<=1)
		return 1;
	return foo(--n);
}

int main(){

int b;
b = foo(3);
return 0;
}
