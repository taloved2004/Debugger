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
#include <map>

#include "elf64.h"

using namespace std;

#define ADDRESS_BOUND 0
#define PERMISIONS 1
#define FD 3
#define INODE 4
#define PATH 5

const std::string WHITESPACE = " \n\r\t\f\v";

unsigned long getEntryPoint(const char *exe_file_name);
unsigned long find_symbol(const char *symbol_name, const char *exe_file_name, int *error_val);
unsigned long get_location_in_got(const char *symbol_name, const char *exe_file_name, int *error_val);

struct BreakPoint {
	unsigned long address;
	unsigned long original_data;
	unsigned long data_trap;
	std::string symbol_name;
};

struct BreakPoint* initateBreakPoint(unsigned long address, unsigned long original_data, unsigned long data_trap, std::string name){
	struct BreakPoint* b = new BreakPoint();
	b->address = address;
	b->original_data = original_data;
	b->data_trap = data_trap;
	b->symbol_name = name;
	return b;
}

class BreakPoints
{
private:
  std::map<unsigned long,BreakPoint*> my_breakPoints;
  pid_t child_pid;
  BreakPoints() : my_breakPoints(), child_pid(){};

public:
  BreakPoints(BreakPoints const &) = delete;     // disable copy ctor
  void operator=(BreakPoints const &) = delete; // disable = operator
  static BreakPoints &getInstance()             // make SmallShell singleton
  {
    static BreakPoints instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~BreakPoints(){
      for (auto& pair : my_breakPoints) {
        delete pair.second;
        pair.second = nullptr;
    }
  }

  void setChildPid(pid_t child_pid){
	  this->child_pid = child_pid;
  }
   pid_t getChildPid() const{
	  return child_pid;
  }
  void addBreakPoint(struct BreakPoint* b){
	  if(!isExists(b->address))
	     my_breakPoints.insert(std::make_pair(b->address, b));
  }
  bool isExists(unsigned long address){
	// Checking if a key exists using find
    auto iterator = my_breakPoints.find(address);
    if(iterator != my_breakPoints.end()){
		return true;
	}
	return false;
  }
  
  struct BreakPoint* getBreakPoint(unsigned long address){
	// Checking if a key exists using find
	auto iterator = my_breakPoints.find(address);
    if(iterator != my_breakPoints.end()){
		return iterator->second;
	}
	return nullptr;
  }
  
  void removeBreakPoints(){
	    for (auto& pair : my_breakPoints) {
			//	get breakpoint data
				BreakPoint * breakPoint = pair.second;
				
			//	restore original data	
				ptrace(PTRACE_POKETEXT, this->child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);
    }
  }
  
  void restoreBreakPoints(){
	  for (auto& pair : my_breakPoints) {
			BreakPoint* breakPoint = pair.second;			
			//	restore breakpoint	
				ptrace(PTRACE_POKETEXT, this->child_pid, (void *)breakPoint->address, (void *)breakPoint->data_trap);
    }
  }
};

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
	if(req=="")
		return;
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

void printEndMsg(int wait_status){
    int exit_code = WEXITSTATUS(wait_status);
    std::cout << "Process ended with exit code: 0x" << exit_code << std::endl;	
}



void interactWithUser(pid_t child_pid){
	//  ask user what he wants to do (with a loop that keeps asking untill he requests 'run')
        std::string req = "";
        while (req != "run")
        {
            std::cout << "[debugger]: ";
            std::getline(std::cin, req);
            handleRequset(req, child_pid);
        }
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


unsigned long getStartingAddress(std::string address_bounderies){
	std::string str ="-";
	 std::size_t found = address_bounderies.find(str);
	int endIndex = -1;
    if (found ==std::string::npos) {
        std::cout << "Error could not find the '-' character" << std::endl;
        return 0;
    }
    unsigned long address = 0;
	std::string start_address = address_bounderies.substr(0, found);
	if(isValidHexaNumber("0x" + start_address)){
		address = convertHexaStringToLong(start_address);	
	}
	else{
		std::cout << "ERROR: could not convert address to a number\n";
		return 0;
	}

	return address;
}

void putBreakPoint(unsigned long address_to_breakPoint, std::string symbol_name){
	BreakPoints &my_breakPoints = BreakPoints::getInstance();
	pid_t child_pid = my_breakPoints.getChildPid();
	
	//	put breakpoint
	unsigned long original_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_to_breakPoint, NULL);
	unsigned long data_trap = (original_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
	ptrace(PTRACE_POKETEXT, child_pid, (void *)address_to_breakPoint, (void *)data_trap);
	
	//	save breakpoint in data structure
	struct BreakPoint* b = initateBreakPoint(address_to_breakPoint,original_data ,data_trap, symbol_name);
	my_breakPoints.addBreakPoint(b);
}


void checkForSymbolAndPutBreakPoint(std::string symbol_name, std::vector<std::string> library){
			int error_val;
			unsigned long function_offset_in_library = find_symbol(symbol_name.c_str(), library[PATH].c_str(),&error_val); 
			if(error_val==1 && function_offset_in_library!=0){
				unsigned long starting_address_of_library = getStartingAddress(library[ADDRESS_BOUND]);
				unsigned long function_address = starting_address_of_library + function_offset_in_library;
				
				//	put breakpoint and save original data in a data structure
				std::string name(symbol_name);
				putBreakPoint(function_address, name);
			}
	
}


void handleFork(){

		int wait_status;
		struct user_regs_struct regs;
	
	     // remove  all breakpoints
	     BreakPoints& my_breakPoints = BreakPoints::getInstance();
	     	

	     my_breakPoints.removeBreakPoints();     
	     
	     pid_t child_pid = my_breakPoints.getChildPid();
	    
	     //	put breakpoint in return address
	     ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
	    
	    unsigned long return_address_from_fork = regs.rsp;
	     putBreakPoint(return_address_from_fork, "fork_return");
	     
	     //---------------------------
		while(regs.rip != return_address_from_fork &&regs.rip != 0 && 1==0){
			//printf("rip: 0x%lx\n", regs.rip);
			ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
			wait(&wait_status);
			ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
		}
		//---------------------------


			     //	continue running and forking
	     ptrace(PTRACE_CONT, child_pid, NULL, NULL);
		 wait(&wait_status);
		
		if(WIFEXITED(wait_status)){
			printEndMsg(wait_status);
			exit(1);
		}
	     //	restore rip and remove breakpoint 
		 	ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
			regs.rip -= 1;
			ptrace(PTRACE_SETREGS, child_pid, 0, &regs);  
       	
	        struct BreakPoint* breakPoint = my_breakPoints.getBreakPoint(regs.rip);
			ptrace(PTRACE_POKETEXT, child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);
	   
	     //	get child pid from rax register
			pid_t forked_child_pid = regs.rax;	     
	     
	     //	attatch to child and remove breakpoint in return address
			if (ptrace(PTRACE_ATTACH, forked_child_pid, NULL, NULL) < 0) {
                perror("ptrace attach child 1");
                exit(1);
            }
            ptrace(PTRACE_POKETEXT, forked_child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);
	     
	     //	restore forked child rip
			ptrace(PTRACE_GETREGS, forked_child_pid, 0, &regs);
			regs.rip -= 1;
			ptrace(PTRACE_SETREGS, forked_child_pid, 0, &regs);  
	     
	     //	continue child running and detach	     
	     ptrace(PTRACE_DETACH, forked_child_pid, NULL, NULL);

	     
	     // put breakpoint in fork for father and all other breakpoints
	     my_breakPoints.restoreBreakPoints();
	     
	
}

void checkSharedLibraries(pid_t child_pid, std::string func_name){
	
	//	get shared libraries from "/proc/[pid]/maps" file
	std::string proc_file_name = "/proc/" + std::to_string(child_pid) + "/maps";
	FILE *maps_file = fopen(proc_file_name.c_str(), "rb");
    if (maps_file == NULL)
    {
        perror("fopen");
        return;
    }
    
    char buffer[4096];
    std::string line;
    
    while (fgets(buffer, sizeof(buffer), maps_file)) {
        size_t len = strlen(buffer);

        // Remove trailing newline if present
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        const char* buffer_const = buffer;
        std::vector<std::string> library = parseRequset(buffer_const); 
        if(library.size()== PATH+1){
			std::string path (library[PATH]);
			if(path.find(".so") != std::string::npos){
				checkForSymbolAndPutBreakPoint(func_name, library);
			}
		}
		
    }	
}


void initiateForkBreakPoint(std::string exe_name){
	
	    //	set child's pid in Data structure
    BreakPoints& my_breakPoints = BreakPoints::getInstance();
	pid_t child_pid = my_breakPoints.getChildPid();
	int error_val;
	unsigned long fork_GOT_address;
	unsigned long fork_plt_address;
	find_symbol("fork", exe_name.c_str(),&error_val);
	if(error_val==-3){ //	that means there is a use of fork in the executable
		int error_val1;
		fork_GOT_address = get_location_in_got("fork", exe_name.c_str(), &error_val1);
		fork_plt_address = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)fork_GOT_address, NULL) - 6;

		putBreakPoint(fork_plt_address, "fork");
	}	
}

void run_debugger(pid_t child_pid, long address_found, bool is_from_shared_library, std::string symbol_name, std::string exe_name, bool has_lazy_binding)
{
    //	vars
    int wait_status;
    struct user_regs_struct regs;
    unsigned long long original_address_found = address_found;
    unsigned long long data_of_function, data_trap_of_function;
    
    //	set child's pid in Data structure
    BreakPoints& my_breakPoints = BreakPoints::getInstance();
	my_breakPoints.setChildPid(child_pid);
   
    //	wait for child to reach first instruction
    waitpid(child_pid, &wait_status, 0);

    //	interact with user
    interactWithUser(child_pid);
    
    //	start until the entry point of the executable
	//	get entry point address
	unsigned long entry_point_address = getEntryPoint(exe_name.c_str());	
			
	//	put breakpoint in entry point
	unsigned long data_of_entry_point = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)entry_point_address, NULL);
	unsigned long data_trap_of_entry_point = (data_of_entry_point & 0xFFFFFFFFFFFFFF00) | 0xCC;
	ptrace(PTRACE_POKETEXT, child_pid, (void *)entry_point_address, (void *)data_trap_of_entry_point);
	
	//	start child running in order to make him do dynamic linking (when BIND_NOW is on)
	ptrace(PTRACE_CONT, child_pid, NULL, NULL);
	wait(&wait_status);
			
	//	check if child has exited (not likely but to make sure)
	if(WIFEXITED(wait_status))
	{
				printEndMsg(wait_status);
				return;
	}	
	//	remove breakpoint and set rip to the start command
	ptrace(PTRACE_POKETEXT, child_pid, (void *)entry_point_address, (void *)data_of_entry_point);
	

	//	point to original command again
	ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
	regs.rip -= 1;
	ptrace(PTRACE_SETREGS, child_pid, 0, &regs);    
    
    //	check all shared libraries if they contain the function
	checkSharedLibraries(child_pid, symbol_name);
	
	//	put breakpoint in fork in order to stop the process before forking
	//initiateForkBreakPoint(exe_name);

    //  -------put breakpoint at address-----------

    //  put breakpoint in the start of the function
    if(!is_from_shared_library){
		putBreakPoint(address_found, symbol_name);
	}
	
    //  start child's program -- currently at entry point
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);

    //  wait for child to stop/finish
    wait(&wait_status);

    while (!WIFEXITED(wait_status))
    {

        //	point to original command again
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        regs.rip -= 1;
        ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
       
       
        //  remove break at starting point by restoring original data
        struct BreakPoint* breakPoint = my_breakPoints.getBreakPoint(regs.rip);
        ptrace(PTRACE_POKETEXT, child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);
		
		if(breakPoint->symbol_name == "fork"){	
			handleFork();
		}
		else{
			//	print stop messege
			std::cout << "Stopped at " << breakPoint->symbol_name << "\n";

			//	interact with user
			interactWithUser(child_pid);
				
			//	do original command of the function
			ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
			wait(&wait_status);
			
			//	return breakpoint 
			ptrace(PTRACE_POKETEXT, child_pid, (void *)breakPoint->address, (void *)breakPoint->data_trap);		
		}
        //  continue child's program
        ptrace(PTRACE_CONT, child_pid, NULL, NULL);

        //  wait for child to stop/finish
        wait(&wait_status);   
    }
    
	printEndMsg(wait_status);
}

