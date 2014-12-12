# Makefile
#
# $Id$
#

OSFLAGS  = -DOS_LINUX -Dextname
CFLAGS   = -g -O2 -fPIC -Wall -Wuninitialized -I. -I$(MIDASSYS)/include -I.. -m64
CXXFLAGS = $(CFLAGS)

LIBS = -lm -lz -lutil -lnsl -lpthread -lrt

# MIDAS library
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a
MFE = $(MIDASSYS)/linux/lib/mfe.o

# location of the Wiener SNMP MIB file
CFLAGS += -DWIENER_MIB_DIR=\"$(PWD)\"

#all:: dox
all:: fewiener.exe
#all:: fewiener_eb.exe
#all:: fefgdwiener.exe
#all:: fedeapvme01.exe
#all:: fedeapvme02.exe
#all:: fedeapvme03.exe
#all:: fedeapmpod.exe

dox: ; doxygen ../Doxyfile 

fewiener_eb.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DUSE_EB

fewiener.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS)

fefgdwiener.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"fefgdwiener\" -DEQ_NAME=\"FgdWiener\" -DEQ_EVID=EVID_FGDWIENER -DUSE_EB

fedeapvme01.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"fedeapvme01\" -DEQ_NAME=\"deapvme01\" -DEQ_EVID=1

fedeapvme02.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"fedeapvme02\" -DEQ_NAME=\"deapvme02\" -DEQ_EVID=1

fedeapvme03.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"fedeapvme03\" -DEQ_NAME=\"deapvme03\" -DEQ_EVID=1

fedeapmpod.exe: %.exe: fewiener.cxx $(MIDASLIBS) $(MFE)
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIBS) -DFE_NAME=\"fedeapmpod\" -DEQ_NAME=\"deapmpod\" -DEQ_EVID=1

mxml.o: $(MIDASSYS)/../mxml/mxml.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

strlcpy.o: $(MIDASSYS)/../mxml/strlcpy.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

#%.cxx.d: %.cxx
#	$(CXX) -MF $<.d -MT $< -MM $(CXXFLAGS) $(OSFLAGS) -c $<
#
#%.c.d: %.c
#	$(CC) -MF $<.d -MT $< -MM $(CFLAGS) $(OSFLAGS) -c $<

depend:

-include *.d

clean::
	-rm -f *.o *.exe
	-rm -f *.d

# end
