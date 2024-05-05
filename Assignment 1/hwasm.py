from enum import Enum
import sys
from pathlib import Path
from typing import Callable
import re

label_map = {}

byte_index = 0

class Directive:
     def __init__(self, name: str, handler: Callable):
          self.name = name
          self.handler = handler

# Populates an expression with values to be computed by python
def populate_expression_with_values(expresion: str) -> str:
     def term_contains_label(term: str) -> bool:
          term = re.sub(r'\(|\)', '', term)

          return term in label_map

     populated_expression = []
     for term in expresion.split():
          if '.' in term:
               populated_expression.append(term.replace('.', str(byte_index)))
          elif term_contains_label(term):
               label = re.sub(r'\(|\)', '', term)
               populated_expression.append(term.replace(label, str(label_map[label])))
          else:
               populated_expression.append(term)

     return ' '.join(populated_expression)

def fill_directive_handler(line: str) -> list[int]:
     repeat, size, value = line.split(', ')


     filler_bytes = []
     for _ in range(eval(populate_expression_with_values(repeat)) * int(size)):
          filler_bytes.append(int(value))

     return filler_bytes

def word_directive_handler(line: str) -> list[int]:
     word = int(line, 16)

     first_byte = word & 0xff
     second_byte = (word >> 8) & 0xff

     return [first_byte, second_byte]

DIRECTIVES_LIST = [
     Directive("code16", lambda *_: []),
     Directive("global", lambda *_: []),
     Directive("fill", fill_directive_handler),
     Directive("word", word_directive_handler)
]

class Instruction:
     def __init__(self, name: str, handler: Callable):
          self.name = name
          self.handler = handler

class Register(Enum):
     AH = 0xb4
     AL = 0xb0

     @staticmethod
     def from_register_name(name: str):
          match name:
               case 'ah': return Register.AH
               case 'al': return Register.AL

          raise ValueError(f"{name} is not a known register.")

def movb_instruction_handler(line: str) -> list[int]:
     byte, register_name = line.split(', ')

     register = Register.from_register_name(register_name[1:])
     byte = byte[1:]

     if byte.startswith('\''):
          byte = ord(byte[1:-1])
     else:
          byte = int(byte, 16)

     return [register.value, byte]

def int_instruction_handler(line: str) -> list[int]:
     interruption = int(line[1:], 16)

     return [0xcd, interruption]

def jmp_instruction_handler(line: str) -> list[int]:
     label_position = label_map[line]
     return [0xeb, label_position - byte_index - 2]

INSTRUCTIONS_LIST = [
     Instruction("movb", movb_instruction_handler),
     Instruction("int", int_instruction_handler),
     Instruction("jmp", jmp_instruction_handler),
     Instruction("hlt", lambda _: [0xf4]),
]

def remove_comments(line: str) -> str:
     return line.split('#')[0].strip()

def is_directive(line: str) -> bool:
     return line.startswith('.')

def is_label(line: str) -> bool:
     return line.endswith(':')

def handle_line(line: str, byte_index: int) -> list[int]:
     if is_directive(line):
          splitted_line = line.split()
          directive_name = splitted_line[0][1:]

          directives = [d for d in DIRECTIVES_LIST if d.name == directive_name]

          if len(directives) == 0:
               raise NotImplementedError(f"Directive {directive_name} not implemented")

          return directives[0].handler(" ".join(splitted_line[1:]))
     elif is_label(line):
          label_map[line[:-1]] = byte_index
     else:
          splitted_line = line.split()
          instruction_name = splitted_line[0]

          instructions = [i for i in INSTRUCTIONS_LIST if i.name == instruction_name]

          if len(instructions) == 0:
               raise NotImplementedError(f"Instruction {instruction_name} not implemented")

          return instructions[0].handler(" ".join(splitted_line[1:]))

     return []

if __name__ == "__main__":
     if len(sys.argv) != 3:
          print("Usage: hwasm [assembly file] [binary file]")
          exit(1)

     asm_file_name = sys.argv[1]
     bin_file_name = sys.argv[2]

     asm_file_lines = Path(asm_file_name).read_text().splitlines()

     with open(bin_file_name, "wb") as bin_file:
          for line in asm_file_lines:
               line = remove_comments(line)

               if line:
                    bytes_to_write = handle_line(line.strip(), byte_index)
               
                    for byte in bytes_to_write:
                         bin_file.write(byte.to_bytes(1, byteorder="big", signed=byte < 0))

                    byte_index += len(bytes_to_write)