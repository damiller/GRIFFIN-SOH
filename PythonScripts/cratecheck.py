#!/usr/bin/env python

import os
import time
import signal
import smtplib
import string, sys

def ping(addresses):
	""" """
	cmd = " ".join(["fping ",addresses])
	output = os.popen(cmd).read().split('\n')
	return output

mail = 1 #set to zero to suppress mailing
debug = 0 # set to one to print debug stuff
#Check if you have to override these by command line arguments
for arg in sys.argv:
  print arg
  if (arg=='nomail'):
    mail=0
    print "Mail will be suppressed \n"
  elif (arg=='debug'):
    debug = 1
    print "Debugging stuff will be printed \n"

thisScript = os.path.abspath(__file__)

XPScrates = "tigxps01 tigxps02 tigxps03 tigxps04 tigxps05 tigxps06 tigxps07 tigxps08 tigxps09 tigxps11 tigvps01 tigvps04"

HVcrates = "tighv00 tighv01 tighv02" 

output = ping(HVcrates)

for crate in XPScrates.split():
  line = ping(crate)[0]
  if debug == 1:
    print line
  if "alive" in line:
    status = os.system("/home1/tigsoh/VME_Code/WIENER_SNMP_LIB/WIENER_SNMP_test/Crate_test %s" % crate)
    line = line + ", status = %d " % status
    if status == 256:
      line = line + "(Power ON)"
    if status == 0:
      line = line + "(Power OFF)"
  output = output + [ line ]

output = output + [ "", thisScript ]

print "Results: \n"

if debug == 1:
  for line in output:
    print line

HOST = "localhost"

FROM = "tigsoh@triumf.ca"

FILE = open('/home1/tigsoh/PythonScripts/email.txt')

TO = FILE.readlines()

SUBJECT = "Crate Status"

BODY = "Crate Status: \n\n%s" % "\n".join(output)

print BODY

body = string.join(("From: %s" % FROM,"To: %s" % TO,"Subject: %s" % SUBJECT,"",BODY), "\r\n")

if mail == 1:
  server = smtplib.SMTP(HOST)
  server.sendmail(FROM, TO, body)
  server.quit()

if debug == 1:
  print body
  

