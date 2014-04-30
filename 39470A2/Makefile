#####################################################################
#
#  Name:         Makefile
#  Created by:   Stefan Ritt
#
#  Contents:     UNIX Makefile for MIDAS slow control frontend
#
#  $Log: Makefile,v $
#  Revision 1.1  1999/12/21 09:39:00  midas
#  Added source
#
#
#####################################################################

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

# This is for Linux ----------------
#LIBS = -lbsd -lm -lutil
LIBS= -lm -lutil -lpthread -lrt
OSFLAGS = -DOS_LINUX # -DOS_CYGWIN

# This is for OSF1 -----------------
#LIBS = -lbsd
#OSFLAGS = -DOS_OSF1

# This is for Ultrix ---------------
#LIBS =
#OSFLAGS = -DOS_ULTRIX -Dextname

# This is for FreeBSD --------------
#LIBS = -lbsd -lcompat
#OSFLAGS = -DOS_FREEBSD

#-------------------------------------------------------------------
# The following lines define direcories. Adjust if necessary
#                 


INC_DIR 	= $(MIDASSYS)/include
LIB_DIR 	= $(MIDASSYS)/linux/lib
DRV_DIR		= .

#-------------------------------------------------------------------
# Drivers needed by the frontend program
#                 
DRIVERS         = rs232.o
####################################################################  

# Lines below here should not be edited
####################################################################

LIB = $(LIB_DIR)/libmidas.a

# compiler
CC = cc
CFLAGS = -g -I$(INC_DIR) -I$(DRV_DIR) $(USERFLAGS)
LDFLAGS =
TARGETS = THERfe mtest

.PHONY: all clean
all:  $(TARGETS)

THERfe:  $(LIB) $(LIB_DIR)/mfe.o frontend.o $(DRIVERS)
	$(CC) -g -o THERfe frontend.o $(LIB_DIR)/mfe.o $(DRIVERS) $(LIB) $(LDFLAGS) $(LIBS)

mtest : mtest.c rs232.c
	gcc -DOS_LINUX $(CFLAGS) -g -o mtest mtest.c rs232.c $(LIB) $(LDFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

clean:
	rm -f *.o *~ \#* $(TARGETS)

