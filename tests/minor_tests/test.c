#include <stdio.h>

int foo( int a){
a++;
return a;
}

int main(){

int b;
b = foo(1);
b = foo(2);

return 43;
}
