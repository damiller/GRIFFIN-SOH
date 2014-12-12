#####################################################################
#
#  Name:         Makefile
#  Created by:   Stefan Ritt
#
#  Contents:     UNIX Makefile for MIDAS slow control frontend
#
#  $Id: Makefile 2779 2005-10-19 13:14:36Z ritt $
#
#####################################################################

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

# This is for Linux ----------------
LIBS = -lbsd -lm -lutil -lpthread -lz

#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#                 
INC_DIR 	= $(MIDASSYS)/include
LIB_DIR 	= $(MIDASSYS)/linux/lib
DRV_DIR		= $(MIDASSYS)/drivers
#-------------------------------------------------------------------
# Drivers needed by the frontend program
#                 
DRIVERS         = hv.o lrs1445a.o rs232.o 

LIB = $(LIB_DIR)/libmidas.a 

# compiler
CC = cc
CFLAGS = -DOS_LINUX -g -Wall -DUNIX -I$(INC_DIR) -I$(DRV_DIR)
LDFLAGS = 

all: felrs1440.exe

felrs1440.exe: $(LIB_DIR)/mfe.o felrs1440.o $(DRIVERS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIB) $(LIBS)

hv.o: $(DRV_DIR)/class/hv.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $< -o $@

#%.o: $(DRV_DIR)/device/%.c
#	$(CC) $(CFLAGS) $(OSFLAGS) -c $< -o $@

nulldev.o: $(DRV_DIR)/device/nulldev.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $< -o $@

%.o: $(DRV_DIR)/bus/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: %.cxx
	$(CXX) $(CFLAGS) $(OSFLAGS) -c $<

clean:
	rm -f *.o *~ \#*

