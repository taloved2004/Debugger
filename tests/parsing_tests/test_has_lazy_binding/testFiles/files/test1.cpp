//g++ test1.cpp -o test1.out -Wl,-z,now

#include <iostream>

void foo(int a){
	a++;
}

int main(){
	foo(2);
	return 0;
}
