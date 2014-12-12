# Makefile
#
# $Id$
#

OSFLAGS  = -DOS_LINUX
CFLAGS   = -g -O2 -fPIC -Wall -Wuninitialized -I. -I$(MIDASSYS)/include -DHAVE_LIBUSB -I$(MIDASSYS)/mscb -I$(MIDASSYS)/drivers/divers -I../shared
CXXFLAGS = $(CFLAGS)

LIBS = -lm -lz -lutil -lnsl -lpthread -lrt

# MIDAS library
CFLAGS += -I$(MIDASSYS)/drivers/vme
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a
MFE = $(MIDASSYS)/linux/lib/mfe.o

#CFLAGS += -DWIENER_MIB_DIR=\"$(PWD)\"

all:: fesnmp.exe
all:: fedeapcdu.exe
all:: fedeapups.exe

fesnmp.exe: %.exe: %.o $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS)

fesnmp_eb.exe: %.exe: fesnmp.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DUSE_EB

fedeapcdu.exe: %.exe: fesnmp.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"deapcdu\" -DEQ_NAME=\"deapcdu\" -DEQ_EVID=1

fedeapups.exe: %.exe: fesnmp.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"deapups\" -DEQ_NAME=\"deapups\" -DEQ_EVID=1

xml.o: $(MIDASSYS)/../mxml/mxml.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

strlcpy.o: $(MIDASSYS)/../mxml/strlcpy.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

%.d: %.cxx
	$(CXX) -MM -MD $(CXXFLAGS) $(OSFLAGS) -c $<

%.d: %.c
	$(CC) -MM -MD $(CFLAGS) $(OSFLAGS) -c $<

depend:

-include fesnmp.d

clean::
	-rm -f *.o *.exe
	-rm -f *.d

# end
