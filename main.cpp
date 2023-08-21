//g++ --std=c++11 -o prf  *.cpp 


#include <iostream>
#include <string.h>


using namespace std;

bool isExec(const char *exe_file_name);
unsigned long find_symbol(const char *symbol_name, const char *exe_file_name, int *error_val);
unsigned long get_location_in_got(const char *symbol_name, const char *exe_file_name, int *error_val);
bool hasLazyBinding(const char *filename);

pid_t run_target(int argc, char **argv);
void run_debugger(pid_t child_pid, long address_found, bool is_from_got, std::string symbol_name,std::string exe_name, bool has_lazy_binding);

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
        string func_name = "";
        while(func_name == ""){
			std::cout << "Choose function to put breakpoint: ";
			std::getline(std::cin, func_name);
			if (func_name == "quit")
				return 0;
		}
		

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
			std::cout<< "syscall failed\n";
			break;
        case -1:
            std::cout << "Could not find such funcion\n";
            std::cout << "Would you like to make a pending breakpoint['y' , 'n']: ";
            
            std::cin >> answer;
            if(answer == "y"){
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
                return 0;

            //  run debugger
            run_debugger(child_pid, function_address, from_shared_library,func_name, exe_file_name, has_lazy_binding);
        }
    }
    return 0;
}
