#include "Utilities.h"


//  ----------- Parsing a cmd_line functions -----------
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

//  ----------- Parsing a cmd_line functions - END -----------

//  ----------- Print functions -----------

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
    if (errno != 0)
        std::cout << "Error: Reading failed\n";
    else
        printf("Data in %s is : 0x%lx\n", address_given.c_str(), data);
}

void printHelpMsg()
{
    printf("\t-------Options:-------\n");
    printf("\t'regs' - print regs' value\n");
    printf("\t'mem 0x***' - print address' value (must be in hexa)\n");
    printf("\t'run' - start/continue process' executing\n");
    printf("\t'quit' - exit the debugger\n");
}

void printStopMsg(std::string symbol_name){
	  std::cout << "Stopped at " << symbol_name << "\n";
}

void printEndMsg(int wait_status)
{
    int exit_code = WEXITSTATUS(wait_status);
    std::cout << "Process ended with exit code: 0x" << exit_code << std::endl;
}

//  ----------- Print functions - END -----------

//  ----------- Handle memory requset functions -----------
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

//  ----------- Handle memory requset functions - END -----------

bool checkArgs(int argc, char *argv[])
{
    //  check amount of arguments
    if (argc != 2)
    {
        cout << "Usage: ./debug \"Path to your executable\"\n";
        return false;
    }
    //  check if file is executable
    string exe_file_name = argv[1];
    std::ifstream file(exe_file_name.c_str());
    if (file.good() == false) {
		cout << "Could not find the file...\n"; 
		return false;
	} 
		
	if(isExec(exe_file_name.c_str()) == false){
		cout << "File is Not executable...\n"; 
		return false;
	}
    return true;
}


std::string getFunctionFromUsr()
{

    string func_name = "";
    while (func_name == "")
    {
        std::cout << "Choose function to put breakpoint: ";
        std::getline(std::cin, func_name);
    }
    return func_name;
}


//	-------------- Parsing symbol's name aux ----------------

// remove the args from the name (for example "foo(int)" ->  "foo")
std::string removeArgsFromString(std::string str){
	size_t pos = str.find('(');
		if (pos != std::string::npos) {
			return str.substr(0, pos);
		} 
		else {
			// If '(' is not found, return the original string.
			return str;
		} 
}

// remove the namespace from the name (for example "std::foo" ->  "foo")
std::string removeNameSpace(std::string str) {
    // Find the last occurrence of "::" in the input string.
    size_t pos = str.rfind("::");

    if (pos != std::string::npos) {
        // If "::" is found, extract the substring starting from that position.
        return str.substr(pos+2);
    } else {
        // If "::" is not found, return the original string.
        return str;
    }
}

//	given a function's name, if it was altered by the compiler, than return real name
std::string demangleSymbol(const char* mangledSymbol) {
    int status;
    char* demangledName = abi::__cxa_demangle(mangledSymbol, nullptr, nullptr, &status);

    if (status == 0) {
        std::string result(demangledName);
        free(demangledName); // Don't forget to free the memory.
		
		//	get only the name of the function
		result = removeArgsFromString(result);
		result = removeNameSpace(result);
		
		return result;
	}
        return "";
}

//	-------------- Parsing symbol's name aux - END ----------------
