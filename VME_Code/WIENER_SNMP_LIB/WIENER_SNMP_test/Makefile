# basic makefile
CXX   = g++
FC		= g77
RM		= rm -f
CC		= gcc-3.3

LD            = gcc-3.3
LDFLAGS       = -O
SOFLAGS       = -shared

#-L/home/zpwu/VME_Code/WIENER_SNMP_LIB
UPDIR           = $(basename $(PWD))/..
CXXLIBDIRS	= -L$(UPDIR)
CXXLIBS		= -lWIENER_SNMP
INCLUDEDIRS 	= -I$(UPDIR)

CXXFLAGS      	=  -Wall -ansi -g $(INCLUDEDIRS)

CFLAGS		+= $(INCLUDEDIRS) $(SOFLAG)
CFLAGS		+= $(CXXLIBDIRS) $(CXXLIBS)
LDFLAGS		= $(CXXFLAGS) -Wl,-rpath,$(UPDIR)

OBJ = ./Crate_test

main: 
	$(CXX) $(CXXFLAGS) CrateTest.cpp $(CXXLIBDIRS) $(CXXLIBS) -o $(OBJ)  
	@echo "Done"



clean:
	$(RM) $(OBJ) 

