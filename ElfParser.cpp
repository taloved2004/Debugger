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

//  for hasLazyBinding
#include <libelf.h>
#include <gelf.h>

#include "elf64.h"

#define ET_NONE 0 // No file type
#define ET_REL 1  // Relocatable file
#define ET_EXEC 2 // Executable file
#define ET_DYN 3  // Shared object file
#define ET_CORE 4 // Core file

/*
 * exe_file_name	- The file to check if executable.
 * return value		- True - if executable. False - otherwise
 */
bool isExec(char *exe_file_name)
{
    //  open elf file
    FILE *file = fopen(exe_file_name, "rb");
    if (file == NULL)
    {
        perror("fopen");
        return false;
    }

    // extract elf header
    Elf64_Ehdr elfHeader;
    if (fread(&elfHeader, sizeof(elfHeader), 1, file) != 1)
    {
        perror("fread");
        fclose(file);
        return false;
    }

    fclose(file);
    //  check type of file
    if (elfHeader.e_type != ET_EXEC)
        return false;
    return true;
}

/* symbol_name		- The symbol (maybe function) we need to search for.
 * exe_file_name	- The file where we search the symbol in.
 * error_val		- If  1: A global symbol was found, and defined in the given executable.
 * 			- If -1: Symbol not found.
 *			- If -2: Only a local symbol was found.
 * 			- If -3: The symbol was found, it is global, but it is not defined in the executable.
 * return value		- The address which the symbol_name will be loaded to, if the symbol was found and is from text section.
 */
unsigned long find_symbol(const char *symbol_name, char *exe_file_name, int *error_val)
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
    int text_section_index;
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
            symtab = curr_section;
        }
        else if (strcmp(name, ".strtab") == 0)
        {
            strtab = curr_section;
        }
        else if (strcmp(name, "text") == 0)
        {
            std::cout << "found text section\n"; // Note: for debug
            text_section_index = i;
        }
    }

    // find the symbol section that matches symbol_name
    int numbers_of_symbols = symtab.sh_size / symtab.sh_entsize;
    char name2[256];
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
            if (curr_symbol.st_shndx == text_section_index && ELF64_ST_BIND(curr_symbol.st_info))
            {
                *error_val = 1;
                fclose(file);
                return curr_symbol.st_value;
            }
            //  if the symbl is from shared library (and therefore mustbe global)
            if (curr_symbol.st_shndx == SHN_UNDEF)
                *error_val = -3;

            //  symbol is local (static function or a variable)
            if (curr_symbol.st_shndx != SHN_UNDEF)
                *error_val = -2;
        }
    }

    // if error wasn't changed- that means we never found the right symbol. so we return -1;
    if ((*error_val) == 0)
        *error_val = -1;

    fclose(file);
    return 0;
}

/* symbol_name		- The symbol we need to search for.
 * exe_file_name	- The file where we search the symbol in.
 * error_val		- If  1: A GOT enterance was found, that will contain the address of the function at run time.
 * 			- If -1: Symbol not found.
 * return value		- The address of the GOT enterance which the symbol_name will be loaded to, if the symbol was found.
 */
unsigned long get_location_in_got(const char *symbol_name, char *exe_file_name, int *error_val)
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

    // extract the dynamic symbol table section, the dynamic string table and the rela.plt header
    Elf64_Shdr dynsym;
    Elf64_Shdr dynstr;
    Elf64_Shdr rela_plt_header;
    char name[256];
    for (int i = 0; i < elfHeader.e_shnum; ++i)
    {
        Elf64_Shdr curr_section;

        //  point to section
        if (fseek(file, elfHeader.e_shoff + i * elfHeader.e_shentsize, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }

        //  read the section
        if (fread(&curr_section, elfHeader.e_shentsize, 1, file) != 1)
        {
            perror("fread");
            fclose(file);
            return 0;
        }

        //  point to the name of the section in the shstrtab
        if (fseek(file, shstrtab.sh_offset + curr_section.sh_name, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }

        //  get the name
        if (fgets(name, sizeof(name), file) == NULL)
        {
            perror("fgets");
            fclose(file);
            return 0;
        }

        //  check if it's relevant
        if (strcmp(name, ".dynsym") == 0)
        {
            dynsym = curr_section;
        }
        else if (strcmp(name, ".dynstr") == 0)
        {
            dynstr = curr_section;
        }
        else if (strcmp(name, ".rela.plt") == 0)
        {
            rela_plt_header = curr_section;
        }
    }

    // find the symbol section that matches symbol_name

    //  get number of entries in the dynmic symol table
    int numbers_of_symbols = dynsym.sh_size / dynsym.sh_entsize;
    Elf64_Sym symbol_in_dynsym;
    int symbol_index = -1;
    char name2[256];
    for (int i = 0; i < numbers_of_symbols; ++i)
    {
        Elf64_Sym curr_symbol;

        //  point to current entry in dynsym table
        if (fseek(file, dynsym.sh_offset + i * dynsym.sh_entsize, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }

        //  read current entry in dynsym table
        if (fread(&curr_symbol, dynsym.sh_entsize, 1, file) != 1)
        {
            perror("fread");
            fclose(file);
            return 0;
        }

        //  point to the actuall address that contains the name of the symbol(meaning in the dynamic str table, not in dym sym table)
        if (fseek(file, dynstr.sh_offset + curr_symbol.st_name, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }

        //  get the symbol name
        if (fgets(name2, sizeof(name2), file) == NULL)
        {
            perror("fgets");
            fclose(file);
            return 0;
        }

        // found a candidate
        if (strcmp(name2, symbol_name) == 0)
        {
            symbol_in_dynsym = curr_symbol;

            //  save index for rela.plt purpose (to identify the symbol)
            symbol_index = i;
        }
    }

    //  if didn't found symbol (for debug purpose)
    if (symbol_index == -1)
    {
        *error_val = -1;
        return 0;
    }

    // search the entry in the rela.plt section for the relocation that associated with the symbol that been searched

    //  get number of entries
    int number_of_entryies_in_rela_plt = rela_plt_header.sh_size / rela_plt_header.sh_entsize;

    Elf64_Rela symbol_rela_entry;
    for (int i = 0; i < number_of_entryies_in_rela_plt; ++i)
    {
        Elf64_Rela curr_rela;
        //  point to current entry in the rela.plt section
        if (fseek(file, rela_plt_header.sh_offset + i * rela_plt_header.sh_entsize, SEEK_SET) != 0)
        {
            perror("fseek");
            fclose(file);
            return 0;
        }

        //  read the current entry in the rela.plt section
        if (fread(&curr_rela, rela_plt_header.sh_entsize, 1, file) != 1)
        {
            perror("fread");
            fclose(file);
            return 0;
        }

        //  check if the r_info field is the same as the index in the dynsym table (meaning that this relocation is the one of the symbol)
        if (symbol_index == ELF64_R_SYM(curr_rela.r_info))
        {
            symbol_rela_entry = curr_rela;
        }
    }

    //  get from rela.plt the address that the actuall adress will be loaded to (this is an address that is in the GOL)
    unsigned long address_in_GOT = symbol_rela_entry.r_offset;

    fclose(file);
    //  we need to return the address of GOT because the relocation hasn't been made yet
    return address_in_GOT;
}

bool hasLazyBinding(const char *filename)
{
    int fd;
    Elf *elf;

    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        perror("ELF library initialization failed.\n");
        return false;
    }

    if ((fd = open(filename, O_RDONLY, 0)) < 0)
    {
        perror("open");
        return false;
    }

    if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
    {
        perror("elf_begin");
        close(fd);
        return false;
    }

    Elf_Scn *scn = NULL;
    GElf_Dyn dyn;
    bool hasBindNow = false;

    while ((scn = elf_nextscn(elf, scn)) != NULL)
    {
        if (gelf_getshdr(scn, &shdr) != &shdr)
        {
            elf_end(elf);
            close(fd);
            return false;
        }

        if (shdr.sh_type == SHT_DYNAMIC)
        {
            Elf_Data *data = elf_getdata(scn, NULL);
            int numDynEntries = data->d_size / shdr.sh_entsize;

            for (int i = 0; i < numDynEntries; ++i)
            {
                gelf_getdyn(data, i, &dyn);

                if (dyn.d_tag == DT_BIND_NOW)
                {
                    hasBindNow = true;
                    break;
                }
            }
        }

        if (hasBindNow)
        {
            break;
        }
    }

    elf_end(elf);
    close(fd);

    return !hasBindNow; // Return true if lazy binding is used, false if not
}