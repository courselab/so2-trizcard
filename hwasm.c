#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_INSTRUCTION_LENGTH 256

// Function to replace the file extension
void replace_extension(char *filename, const char *new_extension)
{
     char *dot = strrchr(filename, '.');
     if (dot)
     {
          *(dot + 1) = '\0';               // Remove existing extension
          strcat(filename, new_extension); // Add new extension
     }
     else
     {
          strcat(filename, new_extension);
     }
}

// Simple function to write bytes to a file
void write_bytes(FILE *fp, const unsigned char *bytes, size_t num_bytes)
{
     fwrite(bytes, 1, num_bytes, fp);
}

// Encode and write MOV, INT, HLT, and JMP instructions
void encode_and_write_instruction(FILE *fp, const char *instruction, const char *operand)
{
     if (strcmp(instruction, "movb") == 0 && operand[3] == 'e')
     {
          unsigned char opcode[] = {0xB4, 0x0E}; // movb $0x0E, %ah
          write_bytes(fp, opcode, sizeof(opcode));
     }
     else if (strcmp(instruction, "movb") == 0)
     {
          unsigned char opcode[] = {0xB0, (unsigned char)strtol(operand + 1, NULL, 0)}; // movb $val, %al
          write_bytes(fp, opcode, sizeof(opcode));
     }
     else if (strcmp(instruction, "int") == 0)
     {
          unsigned char opcode[] = {0xCD, 0x10}; // int $0x10
          write_bytes(fp, opcode, sizeof(opcode));
     }
     else if (strcmp(instruction, "hlt") == 0)
     {
          unsigned char opcode[] = {0xF4}; // hlt
          write_bytes(fp, opcode, sizeof(opcode));
     }
     else if (strcmp(instruction, "jmp") == 0)
     {
          // Simple interpretation for short backward jump to hlt (assuming hlt is the previous instruction)
          unsigned char opcode[] = {0xEB, (unsigned char)strtol(operand, NULL, 0)}; // jmp short (relative address)
          write_bytes(fp, opcode, sizeof(opcode));
     }
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INSTRUCTION_LENGTH 256

// Handle .fill directive
void handle_fill(FILE *fp, int count, int size, int value)
{
     unsigned char byte = value;
     for (int i = 0; i < count * size; i++)
     {
          write_bytes(fp, &byte, 1);
     }
}

// Handle .word directive
void handle_word(FILE *fp, int word)
{
     unsigned char bytes[2] = {word & 0xFF, (word >> 8) & 0xFF};
     write_bytes(fp, bytes, 2);
}

// Parse directive and handle it
void parse_and_handle_directive(FILE *fp, char *line)
{
     int param1, param2, param3;
     if (sscanf(line, ".fill %d, %d, %d", &param1, &param2, &param3) == 3)
     {
          handle_fill(fp, param1, param2, param3);
     }
     else if (sscanf(line, ".word 0x%x", &param1) == 1)
     {
          handle_word(fp, param1);
     }
}

// Function to trim leading spaces
const char* trimLeadingSpaces(const char *str) {
    while(isspace((unsigned char)*str)) str++;
    return str;
}

int main(int argc, char *argv[])
{
     if (argc != 2)
     {
          fprintf(stderr, "Usage: %s <source.S>\n", argv[0]);
          return EXIT_FAILURE;
     }

     char output_filename[256];
     strncpy(output_filename, argv[1], sizeof(output_filename));
     output_filename[sizeof(output_filename) - 1] = '\0'; // Ensure null termination

     // Change the extension from .S to .bin
     replace_extension(output_filename, "bin");

     FILE *fp = fopen(output_filename, "wb");
     if (!fp)
     {
          perror("Failed to open file for writing");
          return EXIT_FAILURE;
     }

     // Open the input file
     FILE *source = fopen(argv[1], "r");
     if (!source)
     {
          perror("Failed to open source file");
          fclose(fp);
          return EXIT_FAILURE;
     }

     char line[MAX_INSTRUCTION_LENGTH];
     while (fgets(line, MAX_INSTRUCTION_LENGTH, source) != NULL)
     {
          char *token = strtok(line, " ,\t\n");
          char instruction[MAX_INSTRUCTION_LENGTH];
          char operand[10] = {0};
          const char *trimmed = trimLeadingSpaces(line);
          if (trimmed[0] == '.')
          { // Check if it's a directive
               parse_and_handle_directive(fp, line);
          }
          else if (token != NULL)
          {
               strcpy(instruction, token);
               token = strtok(NULL, " ,\t\n");
               if (token != NULL)
               {
                    strcpy(operand, token); // Assuming operand is next token
               }
               encode_and_write_instruction(fp, instruction, operand);
          }
     }

     fclose(source);
     fclose(fp);

     printf("Assembled %s successfully to %s\n", argv[1], output_filename);
     return EXIT_SUCCESS;
}
