# Makefile
#
# $Id$
#

OSFLAGS  = -DOS_LINUX -Dextname
CFLAGS   = -g -O2 -fPIC -Wall -Wuninitialized -I. -I$(MIDASSYS)/include -DHAVE_LIBUSB -I$(MIDASSYS)/mscb -I$(MIDASSYS)/drivers/divers -I../shared
#CXXFLAGS = $(CFLAGS) -DHAVE_ROOT -DUSE_ROOT -I$(ROOTSYS)/include
CXXFLAGS = $(CFLAGS)

LIBS = -lm -lz -lutil -lnsl -lpthread -lrt

# MIDAS library
CFLAGS += -I$(MIDASSYS)/drivers/vme
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a
MFE = $(MIDASSYS)/linux/lib/mfe.o

all:: mscbMakeCode.exe fepwrswitch.exe

mscbMakeCode.exe: %.exe: %.o mscb.o mscbrpc.o musbstd.o
	$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ $^ $(MIDASLIBS) $(LIBS) -lusb

fepwrswitch.exe: fepwrswitch.o MscbDevice.o mscb.o mscbrpc.o musbstd.o $(MFE)
	$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ $^ $(MIDASLIBS) $(LIBS) -lusb

mscbdev.o: %.o: $(MIDASSYS)/drivers/device/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -DHAVE_LIBUSB -I$(MIDASSYS)/mscb -c -o $@ -c $<

musbstd.o: %.o: $(MIDASSYS)/drivers/usb/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -DHAVE_LIBUSB -I$(MIDASSYS)/mscb -c -o $@ -c $<

mscbbus.o: %.o: $(MIDASSYS)/drivers/bus/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -DHAVE_LIBUSB -I$(MIDASSYS)/mscb -c -o $@ -c $<

mscbrpc.o mscb.o: %.o: $(MIDASSYS)/mscb/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -DHAVE_LIBUSB -I$(MIDASSYS)/mscb -I$(MIDASSYS)/../mxml -c -o $@ -c $<

mxml.o: $(MIDASSYS)/../mxml/mxml.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

strlcpy.o: $(MIDASSYS)/../mxml/strlcpy.c
	$(CC) $(CFLAGS) $(OSFLAGS) -c -o $@ -c $<

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

%.d: %.cxx
	$(CXX) -MM -MD $(CXXFLAGS) $(OSFLAGS) -I$(ROOTSYS)/include -c $<

%.d: %.c
	$(CC) -MM -MD $(CFLAGS) $(OSFLAGS) -c $<

depend:

-include femscb.d

clean::
	-rm -f *.o *.exe
	-rm -f *.d

# end
