#include "My_Breakpoints.h"

//  constructor for a BreakPoint struct
struct BreakPoint *initateBreakPoint(unsigned long address, unsigned long original_data, unsigned long data_trap, std::string name)
{
    struct BreakPoint *b = new BreakPoint();
    b->address = address;
    b->original_data = original_data;
    b->data_trap = data_trap;
    b->symbol_name = name;
    return b;
}

//  BreakPoints constructor
BreakPoints::BreakPoints() : my_breakPoints(), child_pid(){};

//  BreakPoints D'tor
BreakPoints::~BreakPoints()
{
    for (auto &pair : my_breakPoints)
    {
        delete pair.second;
        pair.second = nullptr;
    }
}

//  set child process that is being traced's pid
void BreakPoints::setChildPid(pid_t child_pid)
{
    this->child_pid = child_pid;
}

//  get child process that is being traced's pid
pid_t BreakPoints::getChildPid() const
{
    return child_pid;
}

//  given a breakpoint struct, adding the breakpoint to the data structue - if it does'nt already exists
void BreakPoints::addBreakPoint(struct BreakPoint *b)
{
    if (!isExists(b->address))
        my_breakPoints.insert(std::make_pair(b->address, b));
}

//  return if a breakpoint in the given address is already exists in the data structure
bool BreakPoints::isExists(unsigned long address)
{
    // Checking if a key exists using find
    auto iterator = my_breakPoints.find(address);
    if (iterator != my_breakPoints.end())
    {
        return true;
    }
    return false;
}

//  given an address, returns the breakpoint structure in the database associated with the address
struct BreakPoint *BreakPoints::getBreakPoint(unsigned long address)
{
    // Checking if a key exists using find
    auto iterator = my_breakPoints.find(address);
    if (iterator != my_breakPoints.end())
    {
        return iterator->second;
    }
    return nullptr;
}

void BreakPoints::removeBreakPointForGood(unsigned long address){
	
	for (auto &pair : my_breakPoints)
    {
        //	get breakpoint data
        BreakPoint *breakPoint = pair.second;
		if(breakPoint->address == address){
			//	restore original data
			ptrace(PTRACE_POKETEXT, this->child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);
			delete breakPoint;
		}
    }
	this->my_breakPoints.erase(address);
}

//  returns the original data to all locaions that were putted breakpoints
void BreakPoints::removeBreakPoints()
{
    for (auto &pair : my_breakPoints)
    {
        //	get breakpoint data
        BreakPoint *breakPoint = pair.second;

        //	restore original data
        ptrace(PTRACE_POKETEXT, this->child_pid, (void *)breakPoint->address, (void *)breakPoint->original_data);
    }
}

//  restore all breakpoints to include the breakpoint
void BreakPoints::restoreBreakPoints()
{
    for (auto &pair : my_breakPoints)
    {
        BreakPoint *breakPoint = pair.second;
        //	restore breakpoint
        ptrace(PTRACE_POKETEXT, this->child_pid, (void *)breakPoint->address, (void *)breakPoint->data_trap);
    }
}
