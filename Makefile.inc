#!/bin/make

ifeq ($(MIDASSYS),)
# use some generic location if it hasn't been defined in the environment
MIDASSYS = $(HOME)/packages/midas-1jun2010
endif
MIDASINCDIR = $(MIDASSYS)/include
MIDASLIBDIR = $(MIDASSYS)/linux/lib
MIDASLIB    = $(MIDASLIBDIR)/libmidas.a
LDMIDASLIBSO = -L$(MIDASLIBDIR) -lmidas-shared
# addeed for latest version ofmidas as of 4 November 2013
MIDAS_SQLITE := $(shell grep -q HAVE_SQLITE $(MIDASSYS)/Makefile && echo 1)
ifdef MIDAS_SQLITE
HAVE_SQLITE := $(shell test -e /usr/include/sqlite3.h && echo 1)
ifdef HAVE_SQLITE
LDMIDASLIBSO += -lsqlite3
endif
endif
# added for midas with MYSQL
MIDAS_MYSQL := $(shell mysql_config --include 2> /dev/null)
ifdef MIDAS_MYSQL
LDFLAGS += $(shell mysql_config --libs) -lodbc
endif
# some magic to determine the architecture of the midas compile
MIDAS32BIT := $(shell ar p $(MIDASLIB) midas.o | file - | grep -q 32 && echo 1)
ifdef MIDAS32BIT
CFLAGS += -m32
CXXFLAGS += -m32
LDFLAGS += -m32
endif
# now determine if strcomb wants const or non const arguments
#   in the case for when they aren't quite right in experim.h for some reason
CONST_STRCOMB := $(shell grep strcomb $(MIDASINCDIR)/midas.h | grep -q const && echo 1)
ifdef CONST_STRCOMB
CFLAGS += -DCONST_STRCOMB
CXXFLAGS += -DCONST_STRCOMB
endif

# ifeq ($(VMEHOME),)
# VMEHOME = $(HOME)/packages/vmisft-7433-3.3/vme_universe
# endif
# VMELIBDIR = $(VMEHOME)/lib
# VMEINCDIR = $(VMEHOME)/include
# LDVMEFLAGS = -L$(VMELIBDIR) -lvme
LDVMEFLAGS = -lvme

ifeq ($(EPICS_DIR),)
EPICS_DIR=/opt/epics
endif
EPICSINCDIR = $(EPICS_DIR)/include
ifeq ($(EPICS_LIB_DIR),)
EPICS_LIB_DIR = $(EPICS_DIR)/lib/linux-x86_64
endif
EPICSDRVDIR = $(MIDASSYS)/drivers/device

ifndef ROOTSYS
ROOTSYS = /opt/root
endif
ROOTCFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS   = $(shell $(ROOTSYS)/bin/root-config --glibs)

ifndef ROOTANADIR
ROOTANADIR = /opt/rootana
endif
ROOTANAINCDIR = $(ROOTANADIR)/include
ROOTANAINTERFACE = $(ROOTANADIR)/libMidasInterface/libMidasInterface.a

BASEDIR=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))

CFLAGS += -I..
CXXFLAGS += -I..
