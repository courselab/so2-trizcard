#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_INSTRUCTION_LENGTH 256
#define MAX_LABELS 128

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

typedef struct
{
     char label[256];
     int address;
} Label;

Label labelTable[MAX_LABELS];
int labelCount = 0;

void addLabel(const char *label, int address)
{
     if (labelCount < MAX_LABELS)
     {
          strcpy(labelTable[labelCount].label, label);
          labelTable[labelCount].address = address;
          labelCount++;
     }
}

Label findLabelAddress(const char *label)
{
     for (int i = 0; i < labelCount; i++)
     {
          if (strstr(labelTable[i].label, label) != NULL)
          {
               return labelTable[i];
          }
     }
     Label notFound;
     notFound.address = -1;
     return notFound; // Label not found
}

// Encode and write MOV, INT, HLT, and JMP instructions
void encode_and_write_instruction(FILE *fp, const char *instruction, const char *operand, int currentAddress)
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
          Label labelResp = findLabelAddress(operand);
          printf("Label: %s, Address: %d\n", labelResp.label, labelResp.address);
          if (labelResp.address == -1)
          {
               fprintf(stderr, "Label '%s' not found\n", operand);
               return;
          }
          int offset = labelResp.address - currentAddress + strlen(labelResp.label) + 1; // for the size of the jmp instruction itself
          unsigned char opcode[] = {0xEB, (unsigned char)(offset & 0xFF)};
          write_bytes(fp, opcode, sizeof(opcode));
     }
     else
     {
          addLabel(instruction, currentAddress);
     }
}

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

void parseAndHandleDirective(FILE *fp, const char *directive, const char *params)
{
     if (strcmp(directive, ".word") == 0)
     {
          unsigned char opcode[] = {0xB0, (unsigned char)strtol(params + 1, NULL, 0)};
          write_bytes(fp, opcode, sizeof(opcode));
          printf("Word: %s\n", opcode);
     }
     else if (strcmp(directive, ".fill") == 0)
     {
          int repeat, size, value;
          sscanf(params, "%d, %d, %d", &repeat, &size, &value);
          for (int i = 0; i < repeat; i++)
          {
               for (int j = 0; j < size; j++)
               {
                    write_bytes(fp, (const unsigned char *)&value, sizeof(char));
               }
          }
          printf("Fill: %d, %d, %d\n", repeat, size, value);
     }
     else
     {
          fprintf(stderr, "Unknown directive: %s\n", directive);
     }
}

// Function to trim leading spaces
const char *trimLeadingSpaces(const char *str)
{
     while (isspace((unsigned char)*str))
          str++;
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

     int currentAddress = 0; // Inicialização de currentAddress
     char line[MAX_INSTRUCTION_LENGTH];
     while (fgets(line, MAX_INSTRUCTION_LENGTH, source) != NULL)
     {
          char *token = strtok(line, " \t\n");
          const char *trimmed = trimLeadingSpaces(line);
          if (token && trimmed[0] == '.')
          {
               printf("Directive: %s\n", token);
               char *directive = token;
               char operand[10] = {0};
               if (token != NULL)
               {
                    strcpy(operand, token); // Assuming operand is next token
               }
               parseAndHandleDirective(fp, directive, operand);
          }
          else if (token)
          {
               char instruction[MAX_INSTRUCTION_LENGTH];
               char operand[10] = {0};
               strcpy(instruction, token);
               token = strtok(NULL, " ,\t\n");
               if (token != NULL)
               {
                    strcpy(operand, token); // Assuming operand is next token
               }
               encode_and_write_instruction(fp, instruction, operand, currentAddress);
          }
          currentAddress += strlen(line);
     }

     fclose(source);
     fclose(fp);

     printf("Assembled %s successfully to %s\n", argv[1], output_filename);
     return EXIT_SUCCESS;
}
