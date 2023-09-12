#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <stdio.h>
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
#include <iostream>
#include <unistd.h>
#include <cxxabi.h>
#include <cstdlib>

#include "elf64.h"
#include "Utilities.h"

#define ET_NONE 0 // No file type
#define ET_REL 1  // Relocatable file
#define ET_EXEC 2 // Executable file
#define ET_DYN 3  // Shared object file
#define ET_CORE 4 // Core file

bool isExec(const char *exe_file_name);
unsigned long search_symbol(Elf64_Shdr symtab, Elf64_Shdr strtab, const char *symbol_name, int *error_val, FILE *file, int text_section_index);
unsigned long find_symbol(const char *symbol_name, const char *exe_file_name, int *error_val);
unsigned long get_location_in_got(const char *symbol_name, const char *exe_file_name, int *error_val);
bool hasLazyBinding(const char *exe_file_name);
unsigned long getEntryPoint(const char *exe_file_name);

#endif
