# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC -g -lm -pthread
LIB_NAME = libmemory_manager.so

ifeq ($(OS), Windows_NT)
	RM = del
	OSNAME = $(OS)
else
	RM = rm -rf
	OSNAME = $(shell uname)
endif

# Source and Object Files
SRC = memory_manager.c linked_list.c
OBJ = $(SRC:.c=.o)

.PHONY: all mmanager list run_tests run_test_mmanager run_test_list clean

# Default target
all: mmanager test_mmanager test_list

# Rule to create the dynamic library
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ) $(CFLAGS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager
mmanager: $(LIB_NAME)

# Build the linked list
list: linked_list.o

# Test target to run the memory manager test program
#test_mmanager: $(LIB_NAME)
#$(CC) test_memory_manager.c -o test_memory_manager -L. -lmemory_manager $(CFLAGS)

# Test target to run the linked list test program
#test_list: $(LIB_NAME)
#$(CC) linked_list.c test_linked_list.c -o test_linked_list -L. libmemory_manager.so $(CFLAGS)
	
#run tests
run_tests: run_test_mmanager run_test_list
	
# run test cases for the memory manager
#run_test_mmanager:
#LD_LIBRARY_PATH=. ./test_memory_manager 2

# run test cases for the linked list
#run_test_list:
#LD_LIBRARY_PATH=. ./test_linked_list 0

# Clean target to clean up build files
clean:
	$(RM) *.o *.so test_memory_manager test_linked_list