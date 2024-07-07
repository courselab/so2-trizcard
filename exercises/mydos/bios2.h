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

#ifndef BIOS2_H
#define BIOS2_H

int __attribute__((fastcall)) kread(char *);

void __attribute__((fastcall)) udelay(unsigned short);

#endif  /* BIOS2_H  */
