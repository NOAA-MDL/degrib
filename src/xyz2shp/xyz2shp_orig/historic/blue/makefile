#!/bin/make
#SHELL = /bin/sh

############################
# DEFAULT Compilers and Compiler flags.
############################
CC = cc
STD_FLAGS = -O -q32 -qchars=signed -qmaxmem=8192

############################
# Link Flags
############################

############################
# Standard Defines
############################
#STD_DEF = -D_UNIX_ -DMEMWATCH -DMEMWATCH_STDIO -DDEBUG
STD_DEF = -D_UNIX_ -DMEMWATCH -DMEMWATCH_STDIO

############################
# Do we want to watch memory?
############################
MEMWATCH = memwatch.o

############################
# Standard Includes
############################
STD_INC = -I./memwatch

############################
# Standard Libraries
############################
STD_LIB = -L/usr/lib -lm

############################
# Project names
############################
PRJ_NAME = xyz2shp

CFLAGS = $(STD_FLAGS) $(STD_DEF) $(STD_INC)

############################
# FILES
############################
C_SOURCES = tendian.c \
            myutil.c \
            myassert.c

H_SOURCES = tendian.h \
            myutil.h \
            myassert.h \
            type.h

C_MAIN = xyz2shp.c

C_OBJECTS = $(C_SOURCES:.c=.o) \
            $(MEMWATCH)

############################
#
# TARGETS
#
############################
all: $(PRJ_NAME)

$(PRJ_NAME): $(C_OBJECTS) $(C_MAIN) $(LIB_DEPENDS) $(H_SOURCES)
	$(CC) $(C_MAIN) $(CFLAGS) $(LD_FLAGS) $(C_OBJECTS) $(STD_LIB) -o $(PRJ_NAME)

install:
	cp $(PRJ_NAME).exe ../../bin

clean:
	rm -f *.o *.bak *.BAK

############################
#
# SUFFIXES
#
############################
memwatch.o: ./memwatch/memwatch.c ./memwatch/memwatch.h
	$(CC) -c $(CFLAGS) ./memwatch/memwatch.c

.c.o : $(H_SOURCES)
	$(CC) -c $(CFLAGS) $<
