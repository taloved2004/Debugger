#include <stdio.h>

int foo( int a){
a++;
return a;
}

int main(){

int b;
b = foo(1);
return 43;
}
