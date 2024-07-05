/*
 *    SPDX-FileCopyrightText: 2021 Monaco F. J. <monaco@usp.br>
 *    SPDX-FileCopyrightText: 2024 Beatriz Cardoso de Oliveira <beatrizco@usp.br>
Heitor Tanoue de Mello <heitortanoue@usp.br>
 *   
 *    SPDX-License-Identifier: GPL-3.0-or-later
 *
 *  This file is a derivative work from SYSeg (https://gitlab.com/monaco/syseg)
 *  and contains modifications carried out by the following author(s):
 *  Beatriz Cardoso de Oliveira <beatrizco@usp.br>
Heitor Tanoue de Mello <heitortanoue@usp.br>
 */

/* This source file implements the kernel entry function 'kmain' called
   by the bootloader, and the command-line interpreter. Other kernel functions
   were implemented separately in another source file for legibility. */

#include "bios1.h"		/* For kwrite() etc.            */
#include "bios2.h"		/* For kread() etc.             */
#include "kernel.h"		/* Essential kernel functions.  */
#include "kaux.h"		/* Auxiliary kernel functions.  */

/* Kernel's entry function. */

void kmain(void)
{
  int i, j;
  
  register_syscall_handler();	/* Register syscall handler at int 0x21.*/

  splash();			/* Uncessary spash screen.              */

  shell();			/* Invoke the command-line interpreter. */
  
  halt();			/* On exit, halt.                       */
  
}

/* Tiny Shell (command-line interpreter). */

char buffer[BUFF_SIZE];
int go_on = 1;

void shell()
{
  int i;
  clear();
  kwrite ("hbOS 1.0\n");

  while (go_on)
    {

      /* Read the user input. 
	 Commands are single-word ASCII tokens with no blanks. */
      do
	{
	  kwrite(PROMPT);
	  kread (buffer);
	}
      while (!buffer[0]);

      /* Check for matching built-in commands */
      
      i=0;
      while (cmds[i].funct)
	{
	  if (!strcmp(buffer, cmds[i].name))
	    {
	      cmds[i].funct();
	      break;
	    }
	  i++;
	}

      /* If the user input does not match any built-in command name, just
	 ignore and read the next command. If we were to execute external
	 programs, on the other hand, this is where we would search for a 
	 corresponding file with a matching name in the storage device, 
	 load it and transfer it the execution. Left as exercise. */
      
      if (!cmds[i].funct)
	kwrite ("Command not found\n");
    }
}


/* Array with built-in command names and respective function pointers. 
   Function prototypes are in kernel.h. */

struct cmd_t cmds[] =
  {
    {"help",    f_help},     /* Print a help message.       */
    {"quit",    f_quit},     /* Exit TyDOS.                 */
    {"list",    f_list},     /* List all files in the disk. */
    {"exec",    f_exec},     /* Execute an example program. */
    {0, 0}
  };

struct fsHeader *get_fsHeader() {
  return (struct fsHeader *)0x7c00;
}

/* Build-in shell command: help. */

void f_help()
{
  kwrite ("...me, Obi-Wan, you're my only hope!\n\n");
  kwrite ("   But we can try also some commands:\n");
  kwrite ("      list    (to list all the files in hbOS)\n");
  kwrite ("      exec    (to execute an user program example\n");
  kwrite ("      quit    (to exit hbOS)\n");
}

void f_quit()
{
  kwrite ("Program halted. Bye.");
  go_on = 0;
}

/* Built-in shell command: example.

   Execute an example user program which invokes a syscall.

   The example program (built from the source 'prog.c') is statically linked
   to the kernel by the linker script (tydos.ld). In order to extend the
   example, and load and external C program, edit 'f_exec' and 'prog.c' choosing
   a different name for the entry function, such that it does not conflict with
   the 'main' function of the external program.  Even better: remove 'f_exec'
   entirely, and suppress the 'example_program' section from the tydos.ld, and
   edit the Makefile not to include 'prog.o' and 'libtydos.o' from 'tydos.bin'.

  */


void load_disk_into_memory(int sector_coordinate, int sectors_to_read, void *target_addres) {
  __asm__ volatile(
      "pusha \n"
      "mov boot_drive, %%dl \n"    /* Select the boot drive (from rt0.o). */
      "mov $0x2, %%ah \n"          /* BIOS disk service: op. read sector. */
      "mov %[sectToRead], %%al \n" /* How many sectors to read          */
      "mov $0x0, %%ch \n"          /* Cylinder coordinate (starts at 0).  */
      "mov %[sectCoord], %%cl \n"  /* Sector coordinate   (starts at 1).  */
      "mov $0x0, %%dh \n"          /* Head coordinate     (starts at 0).      */
      "mov %[targetAddr], %%bx \n" /* Where to load the file system (rt0.o).   */
      "int $0x13 \n"               /* Call BIOS disk service 0x13.        */
      "popa \n" ::
          [sectCoord] "g"(sector_coordinate),
      [sectToRead] "g"(sectors_to_read),
      [targetAddr] "g"(target_addres));
}

void f_list() {
  struct fsHeader *fs_header = get_fsHeader();

  // finds sector coordinate of the directory and how many sectors to read
  int sector_coordinate = 1 + fs_header->bootSectors; 
  int sectors_to_read = fs_header->fileEntries * DIR_ENTRY_LEN / SECTOR_SIZE; 

  extern byte _MEM_POOL;
  void *directory_section = (void *)&_MEM_POOL;

  load_disk_into_memory(sector_coordinate, sectors_to_read, directory_section);

  for (int i = 0; i < fs_header->fileEntries; i++) {
    char *file_name = directory_section + i * DIR_ENTRY_LEN;
    if (file_name[0]) {
      kwrite(file_name);
      kwrite("\n");
    }
  }
}

void f_exec() {
  char *binary_file_name = "prog.bin";

  // find the binary file in the directory
  struct fsHeader *fs_header = get_fsHeader();

  int directory_sector_coordinate = 1 + fs_header->bootSectors;
  int sectors_to_read = fs_header->fileEntries * DIR_ENTRY_LEN / SECTOR_SIZE + 1;

  int memoryOffset = fs_header->fileEntries * DIR_ENTRY_LEN - (sectors_to_read - 1) * 512;

  extern byte _MEM_POOL;
  void *directory_section = (void *)&_MEM_POOL;

  load_disk_into_memory(directory_sector_coordinate, sectors_to_read, directory_section);

  int bin_sector_coordinate;
  for (int i = 0; i < fs_header->fileEntries; i++) {
    char *file_name = directory_section + i * DIR_ENTRY_LEN;
    if (!strcmp(file_name, binary_file_name)) {
      bin_sector_coordinate = directory_sector_coordinate + sectors_to_read + fs_header->sizeMax * i - 1;
      break;
    }
  }

  void *program = (void *)(USER_PROGRAM_START_ADDR);
  void *program_sector_start = program - memoryOffset;

  load_disk_into_memory(bin_sector_coordinate, fs_header->sizeMax, program_sector_start);

  __asm__ volatile(
      "call get_return_addr_into_ebx \n"  // coloca o return address em ebx

      "push %%ebx \n"  // colocar o ebx na stack

      "jmp *%[progAddr] \n"  // jump pra main

      "get_return_addr_into_ebx: \n"
      "  mov (%%esp), %%ebx \n"  // coloca o topo da stack em ebx
      "  add $17, %%ebx \n"      // soma 17 pq são 17 bytes entre o push do ebx na stack até o retorno da f_exec
      "  ret \n"

      ::[progAddr] "r"(program));
}

