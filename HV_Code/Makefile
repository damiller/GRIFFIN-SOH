#!/bin/make

CAENBASEDIR=/opt/CAENHVWrapper

CFLAGS  += -DUNIX -I$(CAENBASEDIR)/include
LDFLAGS += -L$(CAENBASEDIR)/lib/x64
LDLIBS  += -lcaenhvwrapper -lrt -ldl -lm

HV_Control: HV_Control.c 	

