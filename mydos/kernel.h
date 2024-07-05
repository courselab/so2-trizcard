/*
 *    SPDX-FileCopyrightText: 2024 Beatriz Cardoso de Oliveira <beatrizco@usp.br>
Heitor Tanoue de Mello <heitortanoue@usp.br>
 *    SPDX-FileCopyrightText: 2024 Monaco F. J. <monaco@usp.br>
 *   
 *    SPDX-License-Identifier: GPL-3.0-or-later
 *
 *  This file is a derivative work from SYSeg (https://gitlab.com/monaco/syseg)
 *  and contains modifications carried out by the following author(s):
 *  Beatriz Cardoso de Oliveira <beatrizco@usp.br>
Heitor Tanoue de Mello <heitortanoue@usp.br>
 */

#ifndef KERNEL_H
#define KERNEL_H

/* This is kernel's entry function, which is called by the bootloader
   as soon as it loads the kernel from the this image. */

void kmain(void);		

/* This is the command interpreter, which is invoked by the kernel as
   soon as the boot is complete. 
   
   Our tiny command-line parser is too simple: commands are ASCII single words
   with no command line arguments (no blanks). */


void shell();			/* Command interpreter. */
#define BUFF_SIZE 64		/* Max command length.  */
#define PROMPT "> "		/* Command-line prompt. */
#define FS_SIGLEN 4      /* Signature length.                        */
#define DIR_ENTRY_LEN 32 /* Max file name length in bytes.           */
#define SECTOR_SIZE 512

/* The file header. */

struct fsHeader {
  unsigned char signature[FS_SIGLEN];     /* File system signature.                    */
  unsigned short totalSectors;            /* Number of 512-byte disk blocks.           */
  unsigned short bootSectors;             /* Sectors reserved for boot code.           */
  unsigned short fileEntries;             /* Maximum number of files in the disk.      */
  unsigned short sizeMax;                 /* Maximum size of a file in blocks.         */
  unsigned int unused_space;              /* Remaining space less than sizeMax.        */
} __attribute__((packed));                /* Disable alignment to preserve offsets.    */

struct fsHeader *get_fsHeader();


void load_disk_into_memory(int sector_coordinate, int sectors_to_read, void *target_addres);


/* Built-in commands. */

void f_help();
void f_list();
void f_exec();
void f_quit();

extern struct cmd_t
{
  char name[32];
  void (*funct)();
} cmds[];

#endif  /* KERNEL_H  */
