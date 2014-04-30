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

addresses = "tigxps01 tigxps02 tigxps03 tigxps04 tigxps05 tigxps06 tigxps07 tigxps08 tigxps09 tigvps01 tigvps03 tighv00 tighv01 tighv02" 

output = ping(addresses)

print "Results: \n"

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

server = smtplib.SMTP(HOST)
server.sendmail(FROM, TO, body)
server.quit()


