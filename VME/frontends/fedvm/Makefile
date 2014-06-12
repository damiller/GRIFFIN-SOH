# Makefile
#
# $Id$
#

MIDASSYS=$(HOME)/packages/midas

OSFLAGS  = -DOS_LINUX -Dextname
CFLAGS   = -g -O2 -fPIC -Wall -Wuninitialized -I. -I$(MIDASSYS)/include
#CXXFLAGS = $(CFLAGS) -DHAVE_ROOT -DUSE_ROOT -I$(ROOTSYS)/include
CXXFLAGS = $(CFLAGS)

LIBS = -lm -lz -lutil -lnsl -lpthread -lrt

# MIDAS library
CFLAGS += -I$(MIDASSYS)/drivers/vme
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a
MFE = $(MIDASSYS)/linux/lib/mfe.o

# ROOT library
ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs) -lThread -Wl,-rpath,$(ROOTSYS)/lib

# ROOT analyzer library
### ROOTANA = /home/fgddaq/packages/rootana
### CFLAGS += -I$(ROOTANA) -DHAVE_MIDASfedvm.cxx
### ROOTGLIBS += -lXMLParser

all: fedvm.exe

dox: ; doxygen

install: dox

fedvm.exe: %.exe: %.o $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS)

fegasdvm.exe: %.exe: %.o $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS)

#strlcpy.o: $(MIDASSYS)/../mxml/strlcpy.c
#	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: $(MIDASSYS)/drivers/vme/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: $(MIDASSYS)/drivers/vme/vmic/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

client_main.o: %.o: %.c
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -I$(ROOTSYS)/include -c $<

clean::
	-rm -f *.o *.exe

# end
