#include <stdio.h>

int foo( int a){
a++;
return a;
}

int bar(int a){
return foo(++a);
}

int main(){

int b;
b = bar(1);
return 43;
}
