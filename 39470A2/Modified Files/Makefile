
LIBS= -lm -lutil -lpthread
OSFLAGS = -DOS_LINUX # -DOS_CYGWIN

CC = cc
CFLAGS = -g -I$(INC_DIR) -I$(DRV_DIR)
LDFLAGS =

LIB = $(LIB_DIR)/libmidas.a

INC_DIR 	= /home/midas/packages/midas/include
LIB_DIR 	= /home/midas/packages/midas/linux/lib
DRV_DIR		= .

mtest: mtest2.c rs232.c
	gcc -DOS_LINUX $(CFLAGS) -g -o mtest2 mtest2.c rs232.c $(LIB) $(LDFLAGS) $(LIBS)
