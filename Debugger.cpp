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
#include <sstream>

#include "elf64.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

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

string _ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
    return _rtrim(_ltrim(s));
}

std::vector<std::string> parseRequset(const char *cmd_line)
{
    std::vector<std::string> res;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;)
        res.push_back(s);
    return res;
}

void printRegs(pid_t child_pid)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    std::cout << "\t-----Process's Registers------\n ";
    std::cout << "\tRAX: 0x" << std::hex << regs.rax << std::endl;
    std::cout << "\tOrig RAX: 0x" << std::hex << regs.orig_rax << std::endl;
    std::cout << "\tRBX: 0x" << std::hex << regs.rbx << std::endl;
    std::cout << "\tRCX: 0x" << std::hex << regs.rcx << std::endl;
    std::cout << "\tRDX: 0x" << std::hex << regs.rdx << std::endl;
    std::cout << "\tRDI: 0x" << std::hex << regs.rdi << std::endl;
    std::cout << "\tRSI: 0x" << std::hex << regs.rsi << std::endl;
    
    std::cout << "\tR8:  0x" << std::hex << regs.r8 << std::endl;
    std::cout << "\tR9:  0x" << std::hex << regs.r9 << std::endl;
    std::cout << "\tR10: 0x" << std::hex << regs.r10 << std::endl;
    std::cout << "\tR11: 0x" << std::hex << regs.r11 << std::endl;
    std::cout << "\tR12: 0x" << std::hex << regs.r12 << std::endl;
    std::cout << "\tR13: 0x" << std::hex << regs.r13 << std::endl;
    std::cout << "\tR14: 0x" << std::hex << regs.r14 << std::endl;
    std::cout << "\tR15: 0x" << std::hex << regs.r15 << std::endl;
    std::cout << "\tRBP: 0x" << std::hex << regs.rbp << std::endl;
    std::cout << "\tRSP: 0x" << std::hex << regs.rsp << std::endl;
    std::cout << "\tRIP: 0x" << std::hex << regs.rip << std::endl;
    std::cout << "\tCS:  0x" << std::hex << regs.cs << std::endl;
    std::cout << "\tEFLAGS: 0x" << std::hex << regs.eflags << std::endl;
    std::cout << "\tSS:  0x" << std::hex << regs.ss << std::endl;
    std::cout << "\t----------------------\n ";
}

bool isValidHexaNumber(std::string address)
{
    if (address.size() < 3)
        return false;
    if (address.substr(0, 2) != "0x" && address.substr(0, 2) != "0X")
        return false;
    for (int i = 2; i < (int)address.size(); i++)
    {
        if (!std::isxdigit(address[i]))
        {
            return false;
        }
    }
    return true;
}

unsigned long convertHexaStringToLong(std::string hexa_string)
{
    // Remove "0x" prefix
    std::string stripped_string = hexa_string;
    if (stripped_string.substr(0, 2) == "0x" || stripped_string.substr(0, 2) == "0X")
    {
        stripped_string = stripped_string.substr(2);
    }
    // Convert the stripped string to a long value using std::strtoul
    char *endPtr;
    unsigned long result = std::strtoul(stripped_string.c_str(), &endPtr, 16);

    return result;
}

void printMem(std::vector<std::string> req_vector, pid_t child_pid)
{
    //  check if address was given
    if (req_vector.size() < 2)
    {
        std::cout << "Error: address was not given\n";
        return;
    }

    //  check validation
    std::string address_given = req_vector[1];
    if (!isValidHexaNumber(address_given))
    {
        std::cout << "Error: Address must be in hexa format\n";
        return;
    }
    //  convert the adderss to unsigned long
    unsigned long address = convertHexaStringToLong(address_given);
    if (address == 0)
    {
        std::cout << "Error: Illegal address\n";
        return;
    }
    //  Note: to add a check if the address is readable?

    //  get and print data
    unsigned long data;
    data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address, NULL);
    if(errno != 0)
		std::cout << "Error: Reading failed\n";
	else
    printf("Data in %s is : 0x%lx\n", address_given.c_str(), data);
}

void printHelpMsg(){
	printf("\t-------Options:-------\n");
	printf("\t'regs' - print regs' value\n");
	printf("\t'mem 0x***' - print address' value (must be in hexa)\n");
	printf("'\trun' - start/continue process' executing\n");
	printf("'\tquit' - exit the debugger\n");
	
}

void handleRequset(std::string req, pid_t child_pid)
{

    std::vector<std::string> req_vector = parseRequset(req.c_str());
    
	if(req_vector[0]=="regs")
	     printRegs(child_pid);
	else if(req_vector[0]=="mem")
	     printMem(req_vector, child_pid);
	else if(req_vector[0]=="run")
         std::cout << "Running the process...\n";
    else if(req_vector[0]=="--help")
		 printHelpMsg();
	else if(req_vector[0]=="quit")
	{
		 kill(child_pid, SIGTERM);
		 exit(0);
	}
	else
		 std::cout << "Error: Invalid command\n";

   
}

void run_debugger(pid_t child_pid, long address_found, bool is_from_got, std::string symbol_name, bool has_lazy_binding)
{
    //	vars
    int wait_status;
    bool first_run_flag = true;
    struct user_regs_struct regs;
    unsigned long long original_address_found = address_found;
    unsigned long long data_of_function, data_trap_of_function;
    std::string req;

    //	wait for child to reach first instruction
    waitpid(child_pid, &wait_status, 0);

    //  get request from user
    while (req != "run")
    {
        std::cout << "[debugger]: ";
        std::getline(std::cin, req);
        handleRequset(req, child_pid);
    }

    //  -------put breakpoint at address-----------

    // if the address_found is from GOT and doing lazy binding - get the address of function@plt's first line
    // if from got and not doing lazy binding - get the real address from GOT
    if (is_from_got)
    {
		std::cout << "ENTERED\n";
        if (has_lazy_binding)
        {
            address_found = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL) - 6;
            std::cout<< "has lazy\n";
        }
        else{
			//printf("dataB %lx\n" , address_found);
            address_found = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL);//-6;
		//	std::cout<< "has not lazy\n";
	//		printf("dataA %lx\n" , address_found);
		}
    }

    //  put breakpoint in the start of the function
    data_of_function = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL);
    //printf("data %lx\n" , data_of_function);
    data_trap_of_function = (data_of_function & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void *)address_found, (void *)data_trap_of_function);

    //  start child's program
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);

    //  wait for child to stop/finish
    wait(&wait_status);

    while (!WIFEXITED(wait_status))
    {
		std::cout << "Stopped at " << symbol_name << "\n";
		req = "";
        //  remove break at starting point by restoring original data
        ptrace(PTRACE_POKETEXT, child_pid, (void *)address_found, (void *)data_of_function);

        //	point to original command again
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        regs.rip -= 1;
        ptrace(PTRACE_SETREGS, child_pid, 0, &regs);

        //  update the address found if needed
        if (first_run_flag && is_from_got && has_lazy_binding)
        {
            //	remove breakpoint in function@plt
            ptrace(PTRACE_POKETEXT, child_pid, (void *)address_found, (void *)data_of_function);

            //	get real function's address from GOT's enterance
            address_found = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)original_address_found, NULL);

            //  make sure we will know that we already fixed the variable
            first_run_flag = false;
        }

        //  ask user what he wants to do (with a loop that keeps asking untill he requests 'run')
        while (req != "run")
        {
            std::cout << "[debugger]: ";
            std::getline(std::cin, req);
            handleRequset(req, child_pid);
        }

        //	do original command of the function
        ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
        wait(&wait_status);

        //	save original data and put breakpoint at the start of the function
        data_of_function = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL);
        data_trap_of_function = (data_of_function & 0xFFFFFFFFFFFFFF00) | 0xCC;
        ptrace(PTRACE_POKETEXT, child_pid, (void *)address_found, (void *)data_trap_of_function);

        //  continue child's program
        ptrace(PTRACE_CONT, child_pid, NULL, NULL);

        //  wait for child to stop/finish
        wait(&wait_status);
    }

    int exit_code = WEXITSTATUS(wait_status);
    std::cout << "Process ended with exit code: 0x" << exit_code << std::endl;
}
