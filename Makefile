# Source directories
SRC_DIR := src
BUILD_DIR := build
INSTALL_DIR := /usr/local/bin

# Compiler and flags
CC := gcc
CFLAGS := -ggdb -Wall -Wextra -I include -I $(SRC_DIR)

# Object files
OBJECTS := $(BUILD_DIR)/lasm2_tokenizer.o \
			 $(BUILD_DIR)/lasm2_macro.o \
			 $(BUILD_DIR)/lasm2_tokenreader.o \
			 $(BUILD_DIR)/lasm2_parser.o \
			 $(BUILD_DIR)/lasm2_assembler.o

# Main target
$(BUILD_DIR)/lasm2: $(BUILD_DIR) $(wildcard $(SRC_DIR)/*) $(OBJECTS)
	$(CC) -o $@  $(SRC_DIR)/lasm2.c $(CFLAGS) $(OBJECTS)

# Generic rule for object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(CC) -c $< -o $@ $(CFLAGS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $@

# Phony targets
.PHONY: clean examples

all: clean $(BUILD_DIR)/lasm2 

clean:
	rm -rf $(BUILD_DIR)

install: all
	touch $(INSTALL_DIR)
	cp $(BUILD_DIR)/lasm2 $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/lasm2

# Examples
examples: example_basic_syntax example_expressions example_namespaces example_fibonacci example_6502_addressing

example_%: $(BUILD_DIR)/lasm2 examples/%.l
	./$(BUILD_DIR)/lasm2 examples/$*.l -m 6502 -o $(BUILD_DIR)/$*.out