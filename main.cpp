#include <iostream>
#include "Debugger.h"
#include <string.h>

using namespace std;

int main(int argc, char *argv[])
{
    //  check amount of arguments
    if (argc != 2)
    {
        cout << "Error: Invalid number of arguments\n";
        return 0;
    }
    //  check if file is executable
    string exe_file_name = argv[1];
    if (!isExec(exe_file_name.c_str()))
    {
        cout << "Error: File is not an executable\n";
        return 0;
    }
    while (1)
    {
        //  get function's name to debug
        string func_name;
        std::cout << "Choose function to put breakpoint: ";
        std::cin >> func_name;
        if (func_name == "quit")
            return 0;

        //  get location
        bool is_from_GOT = false;
        bool function_found = false;
        bool has_lazy_binding = false;
        int error_val = 0;
        unsigned long function_address = find_symbol(func_name.c_str(), exe_file_name.c_str(), &error_val);
        switch (error_val)
        {
        case -1:
            std::cout << "Error: Could not find funcion's location\n";
            break;
        case -2:

            int error_val1 = 0;
            function_address = get_location_in_got(func_name.c_str(), exe_file_name.c_str(), &error_val1);

            if (error_val1 == -1)
                std::cout << "GOT address wasn't found\n";
            else if (error_val1 == 0)
                std::cout << "Something went wrong... maybe couldnot find the sections\n";
            else
            {
                is_from_GOT = true;
                function_found = true;
                has_lazy_binding = hasLazyBinding(exe_file_name.c_str());
            }
            break;
        case -3:
            std::cout << "Error: Must give name of global function (Not static function, or a variable)\n";
            break;
        default:
            is_from_GOT = false;
            function_found = true;
            break;
        }
        if (function_found)
        {
            //  create child process for debug
            pid_t child_pid = run_target(argc, argv);
            if (child_pid == 0)
                return 0;

            //  run debugger
            run_debugger(child_pid, function_address, is_from_GOT, has_lazy_binding);
        }
    }
    return 0;
}
