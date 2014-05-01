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

include ../Makefile.inc

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

#-------------------------------------------------------------------
# Drivers needed by the frontend program
#                 
DRIVERS         = rs232.o
####################################################################  

# Lines below here should not be edited
####################################################################

LIB = $(MIDASLIBDIR)/libmidas.a

# compiler
CC = cc
CFLAGS += -g -I. -I$(MIDASINCDIR) $(OSFLAGS)
LDFLAGS =
TARGETS = THERfe mtest

.PHONY: all clean
all:  $(TARGETS)

THERfe: frontend.o $(MIDASLIBDIR)/mfe.o $(DRIVERS) $(LIB)
	$(CC) -g -o $@ $^ $(LDFLAGS) $(LIBS)

mtest : mtest.o rs232.o $(LIB)
	$(CC) -g -o $@ $^ $(LDFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

clean:
	rm -f *.o *~ \#* $(TARGETS)

