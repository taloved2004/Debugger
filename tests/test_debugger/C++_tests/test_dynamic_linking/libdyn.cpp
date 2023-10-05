#include <iostream>

//g++ -shared -fPIC -o libdyn.so libdyn.cpp
//sudo mv libdyn.so /usr/lib/

int internal_func(int a){
return 2;
}

int foo( int a){
internal_func(a);
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
