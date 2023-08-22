#ifndef MY_BREAKPOINTS
#define MY_BREAKPOINTS

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
#include <map>
#include "elf64.h"

struct BreakPoint
{
    unsigned long address;
    unsigned long original_data;
    unsigned long data_trap;
    std::string symbol_name;
};

struct BreakPoint *initateBreakPoint(unsigned long address, unsigned long original_data, unsigned long data_trap, std::string name);

class BreakPoints
{
private:
    std::map<unsigned long, BreakPoint *> my_breakPoints;
    pid_t child_pid;
    BreakPoints();

public:
    BreakPoints(BreakPoints const &) = delete;    // disable copy ctor
    void operator=(BreakPoints const &) = delete; // disable = operator
    static BreakPoints &getInstance()             // make SmallShell singleton
    {
        static BreakPoints instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~BreakPoints();

    //  Aux

    void setChildPid(pid_t child_pid);
    pid_t getChildPid() const;
    void addBreakPoint(struct BreakPoint *b);
    bool isExists(unsigned long address);
    struct BreakPoint *getBreakPoint(unsigned long address);
    void removeBreakPoints();
    void removeBreakPointForGood(unsigned long address);
    void restoreBreakPoints();
};

#endif