// g++ --std=c++11 -o prf  *.cpp

#include <iostream>
#include <string.h>
#include "Debugger.h"
#include "ElfParser.h"
#include "Utilities.h"

using namespace std;



int main(int argc, char *argv[])
{
    if (!checkArgs(argc, argv))
        return 1;

    string exe_file_name = argv[1];

    while (1)
    {
        //  get function's name to debug
        string func_name = getFunctionFromUsr();
        if (func_name == "quit")
            return 0;

        //  get location
        bool from_shared_library = false, function_found = false, has_lazy_binding = false;
        std::string answer = "";
        int error_val = 0;
        unsigned long function_address = find_symbol(func_name.c_str(), exe_file_name.c_str(), &error_val);
        switch (error_val)
        {
        case 0:
            std::cout << "syscall failed\n";
            break;
        case -1:
            std::cout << "Could not find such funcion\n";
            std::cout << "Would you like to make a pending breakpoint['y' , 'n']: ";

			std::getline(std::cin, answer);
            if (answer == "y")
            {
                function_found = true;
                from_shared_library = true;
                has_lazy_binding = hasLazyBinding(exe_file_name.c_str());
            }
            break;
        case -2:
            std::cout << "Error: Multiple functions where found\n";
            break;
        case -3:
            from_shared_library = true;
            function_found = true;
            has_lazy_binding = hasLazyBinding(exe_file_name.c_str());
            break;
        case -4:
            std::cout << "Error: Must give name of a function, not a variable\n";
            break;
        default:
            from_shared_library = false;
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
            run_debugger(child_pid, function_address, from_shared_library, func_name, exe_file_name, has_lazy_binding);
        }
    }
    return 0;
}
