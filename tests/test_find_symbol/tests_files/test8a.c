//gcc tests_files/test7a.c tests_files/test7b.c -o tests_files/test7.out -no-pie

#include <stdio.h>

static void foo(int a){
	a++;
}

int main(){
	foo(2);
	return 0;
}
