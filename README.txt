#Debugger

Hello, and welcome to my debugger!

What will it allow you to do:
1. Given an executable, the program allows you to put a breakpoint in any given function (local or global), as long as the function is defined and has only one definition.
2. While debugging the exe file, you have the option to print the current value of the registers of the running program that's being tracked.
3. While debugging the exe file, you have the option to print the current value in a memory address, as long as it readable (legal and with read permissions).


To compile the debugger - execute the following:
- chmod +x ./compile.sh 
- ./compile

To run the debugger - execute the following:
- ./debug "path_to_your_executable"


Than you will be requsted to enter the name of the function you will want to put a breakpoint in - from your file or from a shared library.
If it's from a shared library, you will be asked to make a pending breakpoint.

When debugging, your options are:
"regs"- printing the registers of the process.
"mem '0x***' "- print the value in address '0x***' in memory.
"run" - start or continue the executable's running.
"quit" - terminate debugging.

To exit at any point:
If at any point you want to exit the debugger, enter 'quit'.
