#Debugger

Hello, and welcome to my debugger!

***important:***
in order for the debugger to work, compile your executable file with the flag
'-no-pie'

for example:
gcc -o "exe_name" "File_name1.c" ... "File_nameX.c" -no-pie

To compile - do:
-chmod +x  ./compile.sh 
- ./compile

to run - execute the following:
./debug "path_to_your_executable"


Than you will be requsted to enter a function's name you will want to put a breakpoint in - in your file or in a shared library.
if it's from a shared library, you will be asked to make a pending breakpoint.


Your Options:
"regs"- printing the registers of the process.
"mem '0x***' "- print the value in addres '0x***' in memory.
"run" - start/continue executable's running.

To exit:
If in any point you will want to exit the debugger - enter 'quit'.