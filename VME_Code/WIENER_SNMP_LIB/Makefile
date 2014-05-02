# basic makefile

.LIBPATTERNS ="'lib%.so lib%.a' "

CXX   = g++
FC		= g77
F90		= f90
RM		= rm -f
CC		= gcc-3.3

LD            = gcc-3.3
LDFLAGS       = -O

CXXLIBDIRS	= 
CXXLIBS		= -lnetsnmp -lcrypto
INCLUDEDIRS 	= 

CXXFLAGS      	= -Wall -ansi -fPIC -g $(INCLUDEDIRS)

CFLAGS		+= $(INCLUDEDIRS) $(SOFLAG)
CFLAGS		+= $(CXXLIBDIRS) $(CXXLIBS)
LDFLAGS		= $(CXXFLAGS)
SOFLAGS         = -shared -Wl,-soname

OBJ = libWIENER_SNMP.so

$(OBJ):  WIENER_SNMP.cpp
	$(CXX) $(SOFLAGS)$@ $(CXXFLAGS) $(CXXLIBS) $(CXXLIBDIRS) -o $@ $^
	@echo "Done"

install:
	@cp $(OBJ) /usr/local/lib/



clean:
	$(RM) $(OBJ) 

