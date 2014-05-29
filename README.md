GRIFFIN-SOH
===========

###Troubleshooting
You'll need to give the midas user permissions to access the serial connection to the agilent; something like 
`sudo usermod -a -G dialout grifsoh`

where `dialout` is the group of the serial port as listed in `/dev`


####Notes
39470A2 has been taken from old tigsoh01

epics directory checked out from standard tigress daq svn

Makefile.inc copied from tigress daq svn r96


