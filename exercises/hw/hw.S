###    SPDX-FileCopyrightText: 2021 Monaco F. J. <monaco@usp.br>
###    SPDX-FileCopyrightText: 2024 Beatriz Cardoso de Oliveira <beatrizco@usp.br>
###    SPDX-FileCopyrightText: 2024 Heitor Tanoue de Mello <heitortanoue@usp.br>
###   
###    SPDX-License-Identifier: GPL-3.0-or-later
###
###    Este arquivo faz parte do SYSeg, disponível em https://gitlab.com/monaco/syseg

     # Inicialização, dizer olá, e parar
	# Montagem NASM, caractere por caractere, manualmente
	
     .code16                  # Definir modo 16-bit
     .global _start			# Definir o ponto de entrada

_start:
	movb $0xe, %ah			# Definir modo teletipo do BIOS

	movb $0x48, %al		# Carregar o código ascii de 'H'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x65, %al 		# Carregar o código ascii de 'e'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x6c, %al		# Carregar o código ascii de 'l'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x6c, %al		# Carregar o código ascii de 'l'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x6f, %al		# Carregar o código ascii de 'o'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x20, %al		# Carregar o código ascii de ' '
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x57, %al		# Carregar o código ascii de 'W'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x6f, %al		# Carregar o código ascii de 'o'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x72, %al		# Carregar o código ascii de 'r'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x6c, %al		# Carregar o código ascii de 'l'
	int $0x10			     # Chamar interrupção do BIOS

	movb $0x64, %al		# Carregar o código ascii de 'd'
	int $0x10			     # Chamar interrupção do BIOS

halt:	
	hlt				     # Parar a máquina
	jmp halt			     # Salvaguarda 

	.fill 510 - (. - _start), 1, 0	# Preencher com zeros
	.word 0xaa55        		     # Boot signature
