// g++ --std=c++11 -o prf  *.cpp

#include <iostream>
#include <string.h>
#include "Debugger.h"
#include "ElfParser.h"
#include "Utilities.h"

using namespace std;

std::string getAnswerToMakePendingBreakpoint(){
	std::string answer;
	std::cout << "Could not find such funcion at the moment...\n";
    while(1){
		std::cout << "Would you like to make a pending breakpoint['y' , 'n']: ";
		std::getline(std::cin, answer);
		if (answer == "y" || answer == "n" || answer == "quit"){
			return answer;
		}
	}
}


int main(int argc, char *argv[])
{
    if (!checkArgs(argc, argv))
        return 1;

    string exe_file_name = argv[1];

	bool is_finished = false;
    while (!is_finished)
    {
        //  get function's name to debug
        string func_name = getFunctionFromUsr();
        if (func_name == "quit")
            is_finished = true;

		if(!is_finished){
			//  get location
			bool from_shared_library = false;
			bool function_found = false;
			bool has_lazy_binding = false;
			std::string answer;

			int error_val = 0;
			unsigned long function_address = find_symbol(func_name.c_str(), exe_file_name.c_str(), &error_val);
			switch (error_val)
			{
			case 0:
				//	syscall error have occured
				break;
			case -1:
				//	could not find such function
				answer = getAnswerToMakePendingBreakpoint();
				if(answer == "quit")
					is_finished == true;
				else if (answer == "y")
					function_found = true;
				break;
			case -2:
				std::cout << "Error: Multiple functions were found....\n";
				break;
			case -3:
				from_shared_library = true;
				function_found = true;
				break;
			case -4:
				//	found a variable and not a function
				answer = getAnswerToMakePendingBreakpoint();
				if(answer == "quit")
					is_finished == true;
				else if (answer == "y")
					function_found = true;
				
				break;
			default:
				function_found = true;
				break;
			}
			if (function_found)
			{
				//  create child process for debug
				pid_t child_pid = run_target(argc, argv);
				if (child_pid == 0)
					return 1;
				
				//  run debugger
				is_finished = run_debugger(child_pid, function_address, func_name, exe_file_name);
			}
		}
    }
    return 0;
}
