#include <iostream>

int foo(int a);
int bar(int a){
return foo(++a);
}

int main(){

int b;
b = bar(1);
b = bar(2);
return 43;
}
