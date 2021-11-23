SHELL := /bin/bash


# ==================================================
# COMMANDS

CC = gcc
RM = rm -f

# ==================================================
# DIRECTORIES

SRC = src


# ==================================================
# TARGETS

EXEC = myshell


# ==================================================
# COMPILATION

all: $(EXEC)

# -- add any dependencies here
%: $(SRC)/%.c
	$(CC) $< -o $@

clean:
	$(RM) $(SRC)/*~ *~

purge: clean
	$(RM) $(addprefix , $(EXEC))
