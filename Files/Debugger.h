#ifndef DEBUGGER_H
#define DEBUGGER_H

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
#include "My_Breakpoints.h"
#include "Utilities.h"
#include "ElfParser.h"

using namespace std;

void advanceOneCommand();
void handleOneStep(pid_t child_pid);
void handleRequset(std::string req, pid_t child_pid);
void interactWithUser(pid_t child_pid);
void putBreakPoint(unsigned long address_to_breakPoint, std::string symbol_name);

//  ----------- Handle shared library functions -----------

unsigned long getStartingAddress(std::string address_bounderies);
void checkForSymbolAndPutBreakPoint(std::string symbol_name, std::vector<std::string> library);
void checkSharedLibraries(pid_t child_pid, std::string func_name);

//  ----------- Handle shared library functions - END -----------

// ---- NOTE ::: for handling fork cases - for later use ----

void handleFork();
void initiateForkBreakPoint(std::string exe_name);

// ------- NOTE --------

//  ------------------  Debugger functions ------------------

pid_t run_target(int argc, char **argv);
void run_debugger(pid_t child_pid, long address_found, bool is_from_shared_library, std::string symbol_name, std::string exe_name, bool has_lazy_binding);

//  ------------------  Debugger functions - END ------------------

#endif
