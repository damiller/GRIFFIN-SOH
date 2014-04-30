#!/usr/bin/env python

import os
import smtplib
import string, sys

HOST = "localhost"

FROM = "zpwu@triumf.ca"

FILE = open('/home/zpwu/Python/email.txt')
TO = FILE.readlines()

SUBJECT = "example.py  Email"

BODY = "This is a test for sending email via python BLAH!"

body = string.join((
    "From: %s" % FROM,
    "To: %s" % TO,
    "Subject: %s" % SUBJECT,
    "",
    BODY), "\r\n")

#server = smtplib.SMTP(HOST)
#server.sendmail(FROM, TO, body)
#server.quit()

cmd = '/home/zpwu/VME_Code/WIENER_SNMP_LIB/WIENER_SNMP_test/Crate_test tigxps07 PowerOn'
os.system(cmd)
