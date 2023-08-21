#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <iostream>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <vector>
#include <fstream>

#include <sstream>

#include "../../elf64.h"


pid_t run_target(int argc, char **argv)
{
    pid_t pid = fork();
    if (pid > 0)
    {
        //  father
        return pid;
    }
    else if (pid == 0)
    {
        //  son
        //  allow tracing of this process
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
        {
            perror("ptrace failed");
            return 0;
        }
        //  replace this program image with the given program
        if (execv(argv[1], &argv[1]) < 0)
        {
            perror("execv failed");
            return 0;
        }
    }
    else
        perror("fork faild");
    
    return 0;
}

void printFileLines(FILE* filePtr) {
    char buffer[4096];
    std::string line;

    while (fgets(buffer, sizeof(buffer), filePtr)) {
        size_t len = strlen(buffer);

        // Remove trailing newline if present
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        std::cout << buffer << std::endl;
    }
}

/*
 * exe_file_name	- The file to get it's entry point address.
 * return value		- The address of the entry point which the xecutable will start running
 */
unsigned long getEntryPoint(const char *exe_file_name)
{
    //  open elf file
    FILE *file = fopen(exe_file_name, "rb");
    if (file == NULL)
    {
        perror("fopen");
		return 0;
    }

    // extract elf header
    Elf64_Ehdr elfHeader;
    if (fread(&elfHeader, sizeof(elfHeader), 1, file) != 1)
    {
        perror("fread");
        fclose(file);
        return 0;
    }

	return elfHeader.e_entry;
}


void find_libraries(pid_t child_pid, std::string symbol_name, std::string exe_name){
	
	int wait_status;
    bool first_run_flag = true;
    struct user_regs_struct regs;
    unsigned long long original_address_found;
    unsigned long long data_of_function, data_trap_of_function;
    
    

    //	wait for child to reach first instruction
    waitpid(child_pid, &wait_status, 0);
	
	unsigned long entry_point_address = getEntryPoint(exe_name.c_str());
			
	//	put breakpoint in entry point
	unsigned long data_of_entry_point = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)entry_point_address, NULL);
	unsigned long data_trap_of_entry_point = (data_of_entry_point & 0xFFFFFFFFFFFFFF00) | 0xCC;
	ptrace(PTRACE_POKETEXT, child_pid, (void *)entry_point_address, (void *)data_trap_of_entry_point);
	
	ptrace(PTRACE_CONT, child_pid, NULL, NULL);
	wait(&wait_status);
			
	//	remove breakpoint and set rip to the start command
	ptrace(PTRACE_POKETEXT, child_pid, (void *)entry_point_address, (void *)data_of_entry_point);

			//	point to original command again
	ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
	regs.rip -= 1;
	ptrace(PTRACE_SETREGS, child_pid, 0, &regs);			
	
	std::string proc_file_name = "/proc/" + std::to_string(child_pid) + "/maps";
	std::cout<< "name: " << proc_file_name <<" \n";
	FILE *maps_file = fopen(proc_file_name.c_str(), "rb");
    if (maps_file == NULL)
    {
        perror("fopen");
        return;
    }
	
	printFileLines(maps_file);
    
    fclose(maps_file);
	
}


int main(int argc, char* argv[])
{
	if(argc!=2){
		return 0;
	}
	std::string func_name;
	
	pid_t child_pid = run_target(argc,argv);
	std::cout << "Enter function's name:" ;
	std::cin >> func_name;
	find_libraries(child_pid,func_name, argv[1]);
	
	
	return 0;
}
