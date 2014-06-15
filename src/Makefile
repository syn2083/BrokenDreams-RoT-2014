# $Id $

# Makefile for Rom24. Works fine on my Debian system.
# You may need to use 'gmake' on BSD systems.

CC = gcc
RM = rm
EXE = BrokenDreams


# Use these two lines to use crypt(), ie on Linux systems.

C_FLAGS =  -w  -ggdb3 -g3 -Wall 
L_FLAGS = -lz -lpthread -lcrypt -ldl -rdynamic -L/usr/include/mysql -lmysqlclient  -llua5.2 -luuid
I_FLAGS = 

# Uncomment these two lines to use plaintext passwords.
# This is how you fix the 'crypt' linking errors!
# C_FLAGS = -Wall $(PROF) -DNOCRYPT -DOLD_RAND
# L_FLAGS = $(PROF)

#IMC2 - Comment out to disable IMC2 support
#IMC = 1
#Code support for Intermud-3. Comment out to disable.
I3 = 1

# Source Files
SRC_FILES := $(wildcard *.c)

ifdef IMC
   O_FILES := imc.o sha256.o $(O_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCROM
endif

ifdef I3
   C_FILES := i3.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DI3 -DI3ROM
endif

# Object Files
OBJ_DIR = obj
OBJ_FILES := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

rom: $(OBJ_FILES)
	$(RM) -f $(EXE)
	$(CC) -o $(EXE) $(OBJ_FILES) $(L_FLAGS) `mysql_config --cflags --libs`
	chmod g+w $(EXE)
	chmod g+w $(OBJ_FILES)

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $(C_FLAGS) $< -o $@

clean:
	$(RM) -f $(OBJ_FILES) $(EXE) *~ *.bak *.orig *.rej