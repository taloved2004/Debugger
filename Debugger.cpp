
#include "Debugger.h"

using namespace std;

void handleRequset(std::string req, pid_t child_pid)
{
    if (req == "")
        return;
    std::vector<std::string> req_vector = parseRequset(req.c_str());

    if (req_vector[0] == "regs")
        printRegs(child_pid);
    else if (req_vector[0] == "mem")
        printMem(req_vector, child_pid);
    else if (req_vector[0] == "run")
        std::cout << "Running the process...\n";
    else if (req_vector[0] == "--help")
        printHelpMsg();
    else if (req_vector[0] == "quit")
    {
        kill(child_pid, SIGTERM);
        exit(0);
    }
    else
        std::cout << "Error: Invalid command\n";
}

void interactWithUser(pid_t child_pid)
{
    //  ask user what he wants to do (with a loop that keeps asking untill he requests 'run')
    std::string req = "";
    while (req != "run")
    {
        std::cout << "[debugger]: ";
        std::getline(std::cin, req);
        handleRequset(req, child_pid);
    }
}

void putBreakPoint(unsigned long address_to_breakPoint, std::string symbol_name)
{
    BreakPoints &my_breakPoints = BreakPoints::getInstance();
    pid_t child_pid = my_breakPoints.getChildPid();

    //	put breakpoint
    unsigned long original_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address_to_breakPoint, NULL);
    unsigned long data_trap = (original_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void *)address_to_breakPoint, (void *)data_trap);

    //	save breakpoint in data structure
    struct BreakPoint *b = initateBreakPoint(address_to_breakPoint, original_data, data_trap, symbol_name);
    my_breakPoints.addBreakPoint(b);
}

//  ----------- Handle shared library functions -----------

unsigned long getStartingAddress(std::string address_bounderies)
{
    std::string str = "-";
    std::size_t found = address_bounderies.find(str);
    int endIndex = -1;
    if (found == std::string::npos)
    {
        perror("Error could not find the '-' character");
        return 0;
    }
    unsigned long address = 0;
    std::string start_address = address_bounderies.substr(0, found);
    if (isValidHexaNumber("0x" + start_address))
    {
        address = convertHexaStringToLong(start_address);
    }
    else
    {
        perror("ERROR: could not convert address to a number\n");
        return 0;
    }

    return address;
}

void checkForSymbolAndPutBreakPoint(std::string symbol_name, std::vector<std::string> library)
{
    int error_val;
    unsigned long function_offset_in_library = find_symbol(symbol_name.c_str(), library[PATH].c_str(), &error_val);
    if (error_val == 1 && function_offset_in_library != 0)
    {
        unsigned long starting_address_of_library = getStartingAddress(library[ADDRESS_BOUND]);
        unsigned long function_address = starting_address_of_library + function_offset_in_library;

        //	put breakpoint and save original data in a data structure
        std::string name(symbol_name);
        putBreakPoint(function_address, name);
    }
}

unsigned long getStartAddressOfFile(std::string exe_name, pid_t child_pid){
    //	get shared libraries from "/proc/[pid]/maps" file
    std::string proc_file_name = "/proc/" + std::to_string(child_pid) + "/maps";
    FILE *maps_file = fopen(proc_file_name.c_str(), "rb");
        if (maps_file == NULL)
    {
        perror("fopen");
        return -1;
    }
	    char buffer[4096];
    std::string line;
    
    fgets(buffer, sizeof(buffer), maps_file);
    size_t len = strlen(buffer);

    // Remove trailing newline if present
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
        len--;
    }
    const char *buffer_const = buffer;
    std::vector<std::string> library = parseRequset(buffer_const);
	return getStartingAddress(library[0]);
}


void checkSharedLibraries(pid_t child_pid, std::string func_name)
{

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

    while (fgets(buffer, sizeof(buffer), maps_file))
    {
        size_t len = strlen(buffer);

        // Remove trailing newline if present
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
            len--;
        }
        const char *buffer_const = buffer;
        std::vector<std::string> library = parseRequset(buffer_const);
        if (library.size() == PATH + 1)
        {
            std::string path(library[PATH]);
            if (path.find(".so") != std::string::npos)
            {
                checkForSymbolAndPutBreakPoint(func_name, library);
            }
        }
    }
}

//  ----------- Handle shared library functions - END -----------

// ---- for handling fork cases - for later use ----

void handleFork()
{

    int wait_status;
    struct user_regs_struct regs;

    // remove  all breakpoints
    BreakPoints &my_breakPoints = BreakPoints::getInstance();

    my_breakPoints.removeBreakPoints();

    pid_t child_pid = my_breakPoints.getChildPid();

    //	put breakpoint in return address
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

    unsigned long return_address_from_fork = regs.rsp;
    putBreakPoint(return_address_from_fork, "fork_return");

    //---------------------------
    while (regs.rip != return_address_from_fork && regs.rip != 0 && 1 == 0)
    {
        // printf("rip: 0x%lx\n", regs.rip);
        ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
        wait(&wait_status);
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    }
    //---------------------------

    //	continue running and forking
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);
    wait(&wait_status);

    if (WIFEXITED(wait_status))
    {
        printEndMsg(wait_status);
        exit(1);
    }
    //	restore rip and remove breakpoint
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);

    struct BreakPoint *breakPoint = my_breakPoints.getBreakPoint(regs.rip);
    ptrace(PTRACE_POKETEXT, child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);

    //	get child pid from rax register
    pid_t forked_child_pid = regs.rax;

    //	attatch to child and remove breakpoint in return address
    if (ptrace(PTRACE_ATTACH, forked_child_pid, NULL, NULL) < 0)
    {
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

void initiateForkBreakPoint(std::string exe_name)
{

    //	set child's pid in Data structure
    BreakPoints &my_breakPoints = BreakPoints::getInstance();
    pid_t child_pid = my_breakPoints.getChildPid();
    int error_val;
    unsigned long fork_GOT_address;
    unsigned long fork_plt_address;
    find_symbol("fork", exe_name.c_str(), &error_val);
    if (error_val == -3)
    { //	that means there is a use of fork in the executable
        int error_val1;
        fork_GOT_address = get_location_in_got("fork", exe_name.c_str(), &error_val1);
        fork_plt_address = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)fork_GOT_address, NULL) - 6;

        putBreakPoint(fork_plt_address, "fork");
    }
}

// ---------------

//  ------------------  Debugger functions ------------------

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
            perror("Error: Cannot execute file\n");
            return 0;
        }
    }
    else
        perror("fork faild");

    return 0;
}

void run_debugger(pid_t child_pid, long address_found, bool is_from_shared_library, std::string symbol_name, std::string exe_name, bool has_lazy_binding)
{
    //	vars
    int wait_status;
    struct user_regs_struct regs;

    unsigned long long data_of_function, data_trap_of_function;

    //	set child's pid in Data structure
    BreakPoints &my_breakPoints = BreakPoints::getInstance();
    my_breakPoints.setChildPid(child_pid);

    //	wait for child to reach first instruction
    waitpid(child_pid, &wait_status, 0);

	//	get start address in memory of the executable
	unsigned long start_address = isExec(exe_name.c_str()) ? 0 : getStartAddressOfFile(exe_name, child_pid);
	if(start_address == -1)
	{
		std::cout << "Error: Could not find starting address of exeutable in memory\n";
		return;
	}
	//	update real location in memory
	address_found += start_address;

    //	interact with user
    interactWithUser(child_pid);

    //	start until the entry point of the executable
    //	get entry point address
    unsigned long entry_point_address = getEntryPoint(exe_name.c_str()) + start_address;

    //	put breakpoint in entry point
    putBreakPoint(entry_point_address, "entry_point");

    //	start child running in order to make him do dynamic linking (when BIND_NOW is on)
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);
    wait(&wait_status);

    //	check if child has exited (not likely but to make sure)
    if (WIFEXITED(wait_status))
    {
        printEndMsg(wait_status);
        return;
    }
    //	remove breakpoint and set rip to the start command
	my_breakPoints.removeBreakPointForGood(entry_point_address);

    //	point to original command again
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);

    //	check all shared libraries if they contain the function
    checkSharedLibraries(child_pid, symbol_name);

    //  -------put breakpoint at address-----------
    //  put breakpoint in the start of the function
    if (!is_from_shared_library)
    {
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
        struct BreakPoint *breakPoint = my_breakPoints.getBreakPoint(regs.rip);
        ptrace(PTRACE_POKETEXT, child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);

        if (breakPoint->symbol_name == "fork")
        {
            //  for later implimention
            handleFork();
        }
        else
        {
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

//  ------------------  Debugger functions - END ------------------
