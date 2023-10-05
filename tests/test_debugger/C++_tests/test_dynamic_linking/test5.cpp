#include <iostream>

int double_req1(int m);
int double_req2(int n);

//double_req1 - 4
//double_req2 - 3
//double_req1 - 2
//double_req2 - 1

//then
//double_req2 - 4
//double_req1 - 3
//double_req2 - 2
//double_req1 - 1 

int main(){

	int b;
	b = double_req1(4);
	b = double_req2(4);

	return 0;
}
