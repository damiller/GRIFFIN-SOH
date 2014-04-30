#!/usr/local/bin/python

import os
import smtplib
import string, sys
import notify

# Parse command line parameters if any

mail = 1 # set to zero to suppress mailing
debug = 0 # set to one to print debug tish

# Determine path of this executable .py file

thisScript = os.path.abspath(__file__)

for arg in sys.argv:
  print arg
  if (arg=='nomail'):
    mail=0
    print "Mail will be suppressed \n"
  elif (arg=='debug'):
    debug = 1
    print "Debugging stuff will be printed \n"


# Notify people that we have a problem 

# Evaluate LD_LIBRARY_PATH 
stati = 'Status:\n-------\nLD_LIBRARY_PATH = '
try:
  ld = os.environ.get("LD_LIBRARY_PATH")
except:
  stati += " Error retreiving it!\n"  
else:
  if ld == None:
    stati += " empty! \n"
#    os.environ.set("LD_LIBARY_PATH",'/usr/local/lib')
  else:
    stati += ld + "\n"

userid = os.getuid()
stati += "Useric = " +`userid` + "\n"

SUBJECT = "CRITICAL: TEMPERATURE HAS REACHED CRITICAL VALUE"
BODY = """
The temperature has reached the CRITICAL value
All systems are shutting off 
"""+thisScript+"\n"

if mail:
  notify.email(SUBJECT,BODY)

if debug:
  print"Mail to be sent SUBJECT:"+SUBJECT
  print BODY

# Now attempt the shutdown itself

#First:  List of crates 
VPScrates =["tigvps01","tigvps04"]  # Do not add tigvps02 because that is Scott's crate

XPScrates = ["tigxps01","tigxps02","tigxps03","tigxps04","tigxps05","tigxps06","tigxps08","tigxps09","tigxps10"]
# Do not add tigxps07 because it's somewhere else

HVcrates = ["tighv00","tighv01","tighv02"]

HVcmd = []
for crate in HVcrates:
  HVcmd.append('/home1/tigsoh/HV_Code/HV_Control %s KillAll' % crate)

XPScmd = []
for crate in XPScrates:
  XPScmd.append('/home1/tigsoh/VME_Code/WIENER_SNMP_LIB/WIENER_SNMP_test/Crate_test %s PowerOff' % crate)

VPScmd = []
for crate in VPScrates:
  VPScmd.append('/home1/tigsoh/VME_Code/WIENER_SNMP_LIB/WIENER_SNMP_test/Crate_test %s PowerOff' % crate)

cmdmasterlist = [XPScmd,VPScmd,HVcmd]

for cmdlist in cmdmasterlist:
  for cmd in cmdlist:
    status = os.system(cmd)
    stati += cmd + " returned " + `status` + "\n"

SUBJECT="Shutdown complete"
BODY = """
Shutdown complete.  Check if anything else needs to be done.
""" +thisScript+"\n\n\nStatus info:"+ stati

if debug:
  print "Mail to be sent SUBJECT:"+SUBJECT
  print BODY

if mail:
  notify.email(SUBJECT,BODY)

print "Done"
