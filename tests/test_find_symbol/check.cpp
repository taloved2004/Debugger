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
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   

#include "../../elf64.h"


unsigned long search_symbol(Elf64_Shdr symtab, Elf64_Shdr strtab, const char* symbol_name, int *error_val, FILE* file, int text_section_index){
		int numbers_of_symbols = symtab.sh_size / symtab.sh_entsize;

		char name2[256];
		unsigned long possibe_address;
		int count_locals = 0;
		
		for (int i = 0; i < numbers_of_symbols; ++i)
		{
			Elf64_Sym curr_symbol;
			if (fseek(file, symtab.sh_offset + i * symtab.sh_entsize, SEEK_SET) != 0)
			{
				perror("fseek");
				fclose(file);
				return 0;
			}
			if (fread(&curr_symbol, symtab.sh_entsize, 1, file) != 1)
			{
				perror("fread");
				fclose(file);
				return 0;
			}

			if (fseek(file, strtab.sh_offset + curr_symbol.st_name, SEEK_SET) != 0)
			{
				perror("fseek");
				fclose(file);
				return 0;
			}
			if (fgets(name2, sizeof(name2), file) == NULL)
			{
				perror("fgets");
				fclose(file);
				return 0;
			}
			// found a candidate
			if (strcmp(name2, symbol_name) == 0)
			{
				// if defined in the **text section** of the file and global- finish
				if (curr_symbol.st_shndx != SHN_UNDEF)
				{
						if(curr_symbol.st_shndx == text_section_index) // in text - meaning a function
						{	
							if(ELF64_ST_BIND(curr_symbol.st_info))// global
							{					
								//	found a global function in text section
								*error_val = 1;
								fclose(file);
								return curr_symbol.st_value;
							}
							else
							{
								// its a static function
								if(count_locals++==0)
								{
									possibe_address = curr_symbol.st_value;
								}
							}
						}						
						else{
							// this is a variable
							*error_val = -4;
						}
					}
				//  if the symbl is from shared library (and therefore mustbe global)
				if (curr_symbol.st_shndx == SHN_UNDEF)
					*error_val = -3;
			}
		}
		if(count_locals==1){
			*error_val = 1;
			fclose(file);
			return possibe_address;
		}
		if(count_locals>=2)
			*error_val=-2;
		
		// if error wasn't changed- that means we never found the right symbol. so we return -1;
		if ((*error_val) == 0)
			*error_val = -1;

		fclose(file);
		return 0;
}

unsigned long find_symbol(const char *symbol_name, const char *exe_file_name, int *error_val)
{
    *error_val = 0;
    FILE *file = fopen(exe_file_name, "rb");
    if (file == NULL)
    {
        perror("fopen");
        return 0;
    }

    // extract elf header
    Elf64_Ehdr elfHeader;
    if (fread(&elfHeader, sizeof(elfHeader), 1, file) != 1)
    {
        perror("fread");
        fclose(file);
        return 0;
    }

    // extract the section header string table - from the elf header
    Elf64_Shdr shstrtab;
	
    if (fseek(file, elfHeader.e_shoff + (elfHeader.e_shstrndx * elfHeader.e_shentsize), SEEK_SET) != 0)
    {
        perror("fseek");
        fclose(file);
        return 0;
    }

    if (fread(&shstrtab, elfHeader.e_shentsize, 1, file) != 1)
    {
        perror("fread");
        fclose(file);
        return 0;
    }

    // extract the symbol table section and string table
    Elf64_Shdr symtab;
    Elf64_Shdr strtab;
    Elf64_Shdr dynstr;
    Elf64_Shdr dynsym;
    bool  found_symtab = false;
    bool  found_strtab = false;
    bool  found_dynstr = false;
    bool  found_dynsym = false;
    
    int text_section_index=-1;
    char name[256];
    for (int i = 0; i < elfHeader.e_shnum; ++i)
    {
		Elf64_Shdr curr_section;

        if (fseek(file, elfHeader.e_shoff + i * elfHeader.e_shentsize, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }
        if (fread(&curr_section, elfHeader.e_shentsize, 1, file) != 1)
        {
            perror("fread");
            fclose(file);
            return 0;
        }
        

        if (fseek(file, shstrtab.sh_offset + curr_section.sh_name, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }
        if (fgets(name, sizeof(name), file) == NULL)
        {
            perror("fgets");
            fclose(file);
            return 0;
        }


        if (strcmp(name, ".symtab") == 0)
        
        {
			found_symtab = true;
            symtab = curr_section;
        }
        else if (strcmp(name, ".strtab") == 0)
        {
			found_strtab = true;
            strtab = curr_section;
        }
        else if (strcmp(name, ".text") == 0)
        {
			
            text_section_index = i;
        }
        else if (strcmp(name, ".dynsym") == 0)
        {
			found_dynsym = true;
            dynsym = curr_section;
        }
        else if (strcmp(name, ".dynstr") == 0)
        {
			found_dynstr = true;
            dynstr = curr_section;
        }
	}
        

	
    int numbers_of_symbols;
    // find the symbol section that matches symbol_name
    if(found_symtab && found_strtab){
		return search_symbol(symtab, strtab, symbol_name, error_val, file, text_section_index);
	}
	
	if(!found_dynsym && !found_dynstr){
		fclose(file);
		return 0;
	}
	return search_symbol(dynsym, dynstr, symbol_name, error_val, file, text_section_index);
}

int main(){
	int error_val;
	unsigned long address = find_symbol("foo", "/usr/lib/libtut.so", &error_val);
	
	if(error_val==1)
		printf("return adress = %lx\n", address);
	else
		printf("didn't find ... :(\n");
		
	return 0;
}
