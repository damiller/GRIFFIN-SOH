#!/usr/bin/python

# This file is to be run when the computer reboots.
# It simply sends an e-mail to the mailing list
# defined in notify.py

import os
import smtplib
import string, sys
import notify

# Parse command line parameters if any

mail = 1 # set to zero to suppress mailing
debug = 0 # set to one to print debug shit

for arg in sys.argv:
  print arg
  if (arg=='nomail'):
    mail=0
    print "Mail will be suppressed \n"
  elif (arg=='debug'):
    debug = 1
    print "Debugging stuff will be printed \n"


try:
  hostname = os.environ.get("HOSTNAME")
except:
  hostname = "[Error getting hostname]"  

SUBJECT = "Notice: %s: rebooted" % hostname
BODY = SUBJECT

if mail:
  notify.email(SUBJECT,BODY)

if debug:
  print BODY
  print SUBJECT
