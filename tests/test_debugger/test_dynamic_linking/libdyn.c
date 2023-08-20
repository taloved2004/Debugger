#include <stdio.h>

//gcc -shared -fPIC -o libdyn.so libdyn.c
//sudo mv libdyn.so /usr/lib/


int foo( int a){
a++;
return a;
}

int double_req1(int m);
int double_req2( int n){
	if(n<=1)
		return 1;
	return double_req1(--n);
}

int double_req1(int m){
	if(m<=1)
		return 1;
	return double_req2(--m);
}

int req(int m){
	if(m<=1)
		return 1;
	return req(--m);
}