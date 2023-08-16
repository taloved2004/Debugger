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

#include "elf64.h"

using namespace std;

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
    {
        perror("fork faild");
        return 0;
    }
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
    std::cout << "R15: 0x" << std::hex << regs.r15 << std::endl;
    std::cout << "R14: 0x" << std::hex << regs.r14 << std::endl;
    std::cout << "R13: 0x" << std::hex << regs.r13 << std::endl;
    std::cout << "R12: 0x" << std::hex << regs.r12 << std::endl;
    std::cout << "RBP: 0x" << std::hex << regs.rbp << std::endl;
    std::cout << "RBX: 0x" << std::hex << regs.rbx << std::endl;
    std::cout << "R11: 0x" << std::hex << regs.r11 << std::endl;
    std::cout << "R10: 0x" << std::hex << regs.r10 << std::endl;
    std::cout << "R9:  0x" << std::hex << regs.r9 << std::endl;
    std::cout << "R8:  0x" << std::hex << regs.r8 << std::endl;
    std::cout << "RAX: 0x" << std::hex << regs.rax << std::endl;
    std::cout << "RCX: 0x" << std::hex << regs.rcx << std::endl;
    std::cout << "RDX: 0x" << std::hex << regs.rdx << std::endl;
    std::cout << "RSI: 0x" << std::hex << regs.rsi << std::endl;
    std::cout << "RDI: 0x" << std::hex << regs.rdi << std::endl;
    std::cout << "Orig RAX: 0x" << std::hex << regs.orig_rax << std::endl;
    std::cout << "RIP: 0x" << std::hex << regs.rip << std::endl;
    std::cout << "CS:  0x" << std::hex << regs.cs << std::endl;
    std::cout << "EFLAGS: 0x" << std::hex << regs.eflags << std::endl;
    std::cout << "RSP: 0x" << std::hex << regs.rsp << std::endl;
    std::cout << "SS:  0x" << std::hex << regs.ss << std::endl;
    std::cout << "\t----------------------\n ";
}

bool isValidHexaNumber(std::string address)
{
    if (address.size() < 3)
        return false;
    if (address.substr(0, 2) != "0x" && address.substr(0, 2) != "0X")
        return false;
    for (int i = 2; i < address.size(); i++)
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
    if (strippedString.substr(0, 2) == "0x" || strippedString.substr(0, 2) == "0X")
    {
        strippedString = strippedString.substr(2);
    }
    // Convert the stripped string to a long value using std::strtoul
    char *endPtr;
    unsigned long result = std::strtoul(strippedString.c_str(), &endPtr, 16);

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
        std::cout << "Error: Illegal argument\n";
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
    long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address, NULL);
    printf("Data in %s is : 0x%llx\n", address_given, data);
}

void handleRequset(std::string req, pid_t child_pid)
{
    std::vector<std::string> req_vector = parseRequset(req.c_str());
    switch (req_vector[0])
    {
    case "regs":
        printRegs(child_pid);
        break;
    case "mem":
        printMem(req_vector, child_pid);
        break;
    case "run":
        std::cout << "Running the process...";
        break;
    case "quit":
        //  kill child
        kill(child_pid, SIGTERM);

        // exit process
        exit(0);
    default:
        std::cout << "Error: Invalid command";
        break;
    }
}

void run_debugger(pid_t child_pid, long address_found, bool is_from_GOT, std::string symbol_name, bool has_lazy_binding)
{
    //	vars
    int wait_status, count_runs = 0;
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
        std::cin >> req;
        handleRequset(req);
    }

    //  -------put breakpoint at address-----------

    // if the address_found is from GOT and doing lazy binding - get the address of function@plt's first line
    // if from got and not doing lazy binding - get the real address from GOT
    if (is_from_got)
    {
        if (has_lazy_binding)
            address_found = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL) - 6;
        else
            address_found = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL);
    }

    //  put breakpoint in the start of the function
    data_of_function = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_found, NULL);
    data_trap_of_function = (data_of_function & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void *)address_found, (void *)data_trap_of_function);

    //  start child's program
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);

    //  wait for child to stop/finish
    wait(&wait_status);

    while (!WIFEXITED(wait_status))
    {

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
            std::cin >> req;
            handleRequset(req);
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

    int exit_code = WEXITSTATUS(status);
    std::cout << "Process ended with exit code: " << exit_code << std::endl;
}