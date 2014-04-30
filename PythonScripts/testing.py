#!/usr/bin/env python

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

# Prepare warning message

SUBJECT = "tigsoh INFO:  Tests have started, ignore messages"

BODY = """
Someone has started testing the tigsoh system.
There may be intermittent error messages.
They should be ignored for now.
A later INFO message will indicate that the tests are over
and thaht you should start paying attention to the messages again.
"""+thisScript+"\n"

# And now, notify by e-mail that there's a problem.

if (mail):
  notify.email(SUBJECT,BODY)

if debug:
  print "Mail to be sent SUBJECT:"+SUBJECT
  print BODY

