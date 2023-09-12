

#ifndef UTILITIES_H
#define UTILITIES_H

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
#include <fstream>


#include "ElfParser.h"

#define ADDRESS_BOUND 0
#define PERMISIONS 1
#define FD 3
#define INODE 4
#define PATH 5

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

//  ----------- Parsing a cmd_line functions -----------
string _ltrim(const std::string &s);
string _rtrim(const std::string &s);
string _trim(const std::string &s);
std::vector<std::string> parseRequset(const char *cmd_line);

//  ----------- Parsing a cmd_line functions - END -----------

//  ----------- Print functions -----------

void printRegs(pid_t child_pid);
void printMem(std::vector<std::string> req_vector, pid_t child_pid);
void printHelpMsg();
void printEndMsg(int wait_status);

//  ----------- Print functions - END -----------

//  ----------- Handle memory requset functions -----------

bool isValidHexaNumber(std::string address);
unsigned long convertHexaStringToLong(std::string hexa_string);

//  ----------- Handle memory requset functions - END -----------

//	-------------- Parsing symbol's name aux ----------------

std::string removeArgsFromString(std::string str);
std::string removeNameSpace(std::string str);
std::string demangleSymbol(const char* mangledSymbol);

//	-------------- Parsing symbol's name aux - END ----------------

bool checkArgs(int argc, char *argv[]);
std::string getFunctionFromUsr();



#endif
