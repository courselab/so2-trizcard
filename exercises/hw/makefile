# Compiler and Linker
CC := gcc

# The Target Binary Program
TARGET := hwasm

# Compiler Flags
CFLAGS := -Wall 

# Default target to build the assembler tool
all: $(TARGET)

# Compile the hwasm tool
$(TARGET): hwasm.c
	$(CC) $(CFLAGS) hwasm.c -o $(TARGET)

# Clean up executables
clean:
	rm -f $(TARGET) *.bin *~

# Generic rule for assembling .S files to .bin
%.bin: %.S
	./$(TARGET) $< $@


.PHONY: clean all
