#####################################################################
#
#  Name:         Makefile
#  Created by:   Stefan Ritt
#
#  Contents:     UNIX Makefile for MIDAS EPICS slow control frontend
#
#  $Log: Makefile,v $
#  Revision 1.3  2000/03/03 01:11:22  pierre
#  - Added EPICS_DIR for Epics library
#
#  Revision 1.2  1999/12/21 09:37:38  midas
#  Adjusted for new driver names
#
#  Revision 1.1  1999/09/22 12:24:02  midas
#  *** empty log message ***
#
#
#####################################################################

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

include ../Makefile.inc

# This is for Linux ----------------
LIBS = -lbsd -lm -lutil -lpthread -lrt
# needed for newer versions of EPICS lib
LIBS += -ldl -lreadline -lncurses
# this was added for the latest version of midas as of 4 Nov 2013
LIBS += -lz
OSFLAGS = -DOS_LINUX

# This is for OSF1 -----------------
#LIBS = -lbsd
#OSFLAGS = -DOS_OSF1

# This is for Ultrix ---------------
#LIBS =
#OSFLAGS = -DOS_ULTRIX -Dextname

# This is for FreeBSD --------------
#LIBS = -lbsd -lcompat
#OSFLAGS = -DOS_FREEBSD

DRV_DIR = $(MIDASSYS)/drivers
DRIVERS = generic.o epics_ca.o
####################################################################
# Lines below here should not be edited
####################################################################

LIB = $(MIDASLIBDIR)/libmidas.a $(EPICS_LIB_DIR)/libca.a $(EPICS_LIB_DIR)/libCom.a

# compiler
CC = cc
CFLAGS += -g -I. -I$(MIDASINCDIR) -I$(DRV_DIR)/device -I$(DRV_DIR)/class -I$(EPICSINCDIR) 
# for newer version of epics lib
CFLAGS += -I$(EPICSINCDIR)/os/Linux -I$(EPICSINCDIR)/compiler/gcc -I$(EPICS_DIR)/src/ca/client
#ifeq ($(MIDAS32BIT),1) 
#	CFLAGS += -m32
#	LDFLAGS += -m32
#else
#	CFLAGS += -m64
#	LDFLAGS += -m64
#endif

all: fe_epics

fe_epics:  $(LIB) $(MIDASLIBDIR)/mfe.o frontend.o $(DRIVERS)
	$(CXX) -o fe_epics $^ $(LIB) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: %.cpp
	$(CXX) $(CFLAGS) $(OSFLAGS) -c $<

generic.o: $(DRV_DIR)/class/generic.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

clean:
	rm -f *.o *~ \#* fe_epics
