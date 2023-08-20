//g++ --std=c++11 -o testLazy  *.cpp 


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


//  If a dynmic section hsa an entry that has "DT_BIND_NOW" tag, it means that the binding should be at loading time and not lazy binding
//  So - we will iterate the section and if a section is dynamic, we will search it's entries to see if one of them has this tag
bool hasLazyBinding(const char *exe_file_name)
{
	bool has_dynamic = false;
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
    int numbers_of_entries;
    Elf64_Shdr curr_section;
    for (int i = 0; i < elfHeader.e_shnum; ++i)
    {

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

        if (curr_section.sh_type == SHT_DYNAMIC)
        {
			has_dynamic = true;
            //  iterate
            numbers_of_entries = curr_section.sh_size / curr_section.sh_entsize;
            for (int i = 0; i < numbers_of_entries; ++i)
            {
                Elf64_Dyn curr_entry;
                if (fseek(file, curr_section.sh_offset + i * curr_section.sh_entsize, SEEK_SET) != 0)
                {
                    perror("fseek");
                    fclose(file);
                    return 0;
                }
                if (fread(&curr_entry, curr_section.sh_entsize, 1, file) != 1)
                {
                    perror("fread");
                    fclose(file);
                    return 0;
                }
                if (curr_entry.d_tag == DT_BIND_NOW_TAG_VALUE)
                {
                    return false;
                }
            }
        }
    }
    return has_dynamic;
}


int main(int argc, char* argv[]){
	
	if(argc<2){
		std::cout<< " Input not recieved\n";
	}
	bool has_lazy = hasLazyBinding(argv[1]);
	if(has_lazy)
		std::cout<< " The elf file have lazy binding\n";
	else
		std::cout<< "The elf file DOES NOT have lazy binding\n";

	return 0;
}
