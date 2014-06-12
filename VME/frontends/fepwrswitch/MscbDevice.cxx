//
// MscbDevice.cxx
//

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "MscbDevice.h"

#include "midas.h"
#include "mscb.h"

MscbDevice::MscbDevice(const char* name, int addr)
{
  fTrace = false;
  fAddr = addr;
  fName = strdup(name);
}

MscbDevice::~MscbDevice()
{
  fAddr = 0;
  if (fName)
    free(fName);
  fName = NULL;
}

int MscbDevice::Init(int mscb, const char* nodeName, int numVariables, int svnRevisionMin, int svnRevisionMax, int dataSize)
{
  int status;

  fDataSize = dataSize;

  status = mscb_ping(mscb, fAddr, 0);
  if (status != MSCB_SUCCESS) {
    cm_msg(MERROR, fName, "Init: Cannot ping MSCB address %d", fAddr);
    return FE_ERR_HW;
  }

  status = mscb_info(mscb, fAddr, &fMscbInfo);

  if (status != MSCB_SUCCESS) {
    cm_msg(MERROR, fName, "Init: Get MSCB_INFO from MSCB address %d", fAddr);
    return FE_ERR_HW;
  }

  printf("MSCB device at address %d: %s: proto_ver: %d, vars: %d, name: %s, svn revision: %d\n",
	 fAddr,
	 fName,
	 fMscbInfo.protocol_version,
	 fMscbInfo.n_variables,
	 fMscbInfo.node_name,
         fMscbInfo.svn_revision);

  if (nodeName)
    if (strcmp(fMscbInfo.node_name, nodeName) != 0) {
      cm_msg(MERROR, fName, "Init: MSCB device at address %d is \'%s\' instead of \'%s\'",
	     fAddr, fMscbInfo.node_name, nodeName);
      return FE_ERR_DRIVER;
    }

  if (fMscbInfo.n_variables < numVariables) {
    cm_msg(MERROR, fName, "Init: MSCB device at address %d has wrong number of variables: %d instead of %d",
	   fAddr, fMscbInfo.n_variables, numVariables);
    return FE_ERR_DRIVER;
  }

  if (svnRevisionMin > 0)
    if (fMscbInfo.svn_revision < svnRevisionMin) {
      cm_msg(MERROR, fName, "Init: MSCB device at address %d has unsupported svn revision %d, older than %d",
	     fAddr, fMscbInfo.svn_revision, svnRevisionMin);
      return FE_ERR_DRIVER;
    }

  if (svnRevisionMax > 0)
    if (fMscbInfo.svn_revision > svnRevisionMax) {
      cm_msg(MERROR, fName, "Init: MSCB device at address %d has unsupported svn revision %d, newer than %d",
	     fAddr, fMscbInfo.svn_revision, svnRevisionMax);
      return FE_ERR_DRIVER;
    }

  return MSCB_SUCCESS;
}

int MscbDevice::ShowVariables(int mscb)
{
  printf("%s at MSCB address %d, found %d variables:\n", fName, fAddr, fMscbInfo.n_variables);

  int bcount = 0;
  for (int i=0; i<fMscbInfo.n_variables; i++)
    {
      MSCB_INFO_VAR var_info;
      mscb_info_variable(mscb, fAddr, i, &var_info);
      printf("Var %3d: name \'%-8s\', width %d, unit %3d, prefix %3d, status %3d, flags %d\n",
	     i,
	     var_info.name,
	     var_info.width,
	     var_info.unit,
	     var_info.prefix,
	     var_info.status,
	     var_info.flags);

      bcount += var_info.width;
    }

  printf("Total: %d bytes\n", bcount);

  return MSCB_SUCCESS;
}

int MscbDevice::WriteFloat(int mscb, int ivar, float value)
{
  if (fTrace)
    printf("%s at MSCB address %d: write %f into variable %d\n", fName, fAddr, value, ivar);
  return mscb_write(mscb, fAddr, ivar, &value, 4);
}

int MscbDevice::WriteWord(int mscb, int ivar, uint16_t value)
{
  if (fTrace)
    printf("%s at MSCB address %d: write %d (0x%x) into variable %d\n", fName, fAddr, value, value, ivar);
  return mscb_write(mscb, fAddr, ivar, &value, 2);
}

int MscbDevice::WriteByte(int mscb, int ivar, unsigned char byte)
{
  if (fTrace)
    printf("%s at MSCB address %d: write %d (0x%x) into variable %d\n", fName, fAddr, byte, byte, ivar);
  return mscb_write(mscb, fAddr, ivar, &byte, 1);
}

int MscbDevice::ReadByte(int mscb, int ivar, unsigned char *value)
{
  int size = 1;
  int status = mscb_read(mscb, fAddr, ivar, value, &size);
  assert(size==1);
  if (fTrace)
    printf("%s at MSCB address %d: read %d (0x%x) from variable %d\n", fName, fAddr, *value, *value, ivar);
  return status;
}

int MscbDevice::ReadWord(int mscb, int ivar, uint16_t *value)
{
  int size = 2;
  int status = mscb_read(mscb, fAddr, ivar, value, &size);
  assert(size==1);
  if (fTrace)
    printf("%s at MSCB address %d: read %d (0x%x) from variable %d\n", fName, fAddr, *value, *value, ivar);
  return status;
}

int MscbDevice::ReadFloat(int mscb, int ivar, float *value)
{
  int size = 4;
  int status = mscb_read(mscb, fAddr, ivar, value, &size);
  assert(size==4);
  if (fTrace)
    printf("%s at MSCB address %d: read %f from variable %d\n", fName, fAddr, *value, ivar);
  return status;
}

struct bank_header {
   uint32_t _timestamp;
   uint8_t  _protocol_version;
   uint8_t  _n_variables;
   uint16_t _node_address;
   uint16_t _group_address;
   uint16_t _svn_revision;
};

int MscbDevice::ReadBankSlow(int mscb, void* bank, int bankSize)
{
  unsigned char* data = (unsigned char*)bank;

  time_t now = time(NULL);

  struct bank_header* hbank = (struct bank_header*)bank;
  hbank->_timestamp = now;
  hbank->_protocol_version = fMscbInfo.protocol_version;
  hbank->_n_variables   = fMscbInfo.n_variables;
  hbank->_node_address  = fMscbInfo.node_address;
  hbank->_group_address = fMscbInfo.group_address;
  hbank->_svn_revision  = fMscbInfo.svn_revision;

  int iptr = sizeof(bank_header);
  for (int i=0; i<fMscbInfo.n_variables; i++)
    {
      int size = 4;
      int status = mscb_read(mscb, fAddr, i, data+iptr, &size);

      if (0)
	printf("var %d, iptr %d, status %d, size %d, hex 0x%02x%02x%02x%02x\n", i, iptr, status, size, (data+iptr)[0], (data+iptr)[1], (data+iptr)[2], (data+iptr)[3]);

      if (status != MSCB_SUCCESS)
        {
          memset(bank, 0, bankSize);
          return status;
        }

      assert(size>0);
      assert(size<16);

      if (iptr%size != 0)
	{
	  int shift = size-(iptr%size);
	  //printf("add padding at %d, shift %d\n", iptr, shift);
	  memmove(data+iptr+shift, data+iptr, size);
	  iptr += shift;
	}

      iptr += size;
    }

  return SUCCESS;
}

int MscbDevice::ReadBank(int mscb, void* bank, int bankSize, MscbDeviceBankCopy copyfunc)
{
  char data[10*1024];
  int size = fDataSize;

  time_t now = time(NULL);

  int ivar = 0;
  int iptr = 0;
  while (iptr < fDataSize)
    {
      // Cannot read all data in one go - MSCB complains that data size is > 255 bytes
      //int status = mscb_read_range(mscb, fAddr, 0, FEB64_NUM_VARS-1, data, &size);

      assert(size > 0);

      int xsize = size;
      int xvar = fMscbInfo.n_variables-1;
      if (xsize > 240)
        {
          xsize = 240;
          xvar = ivar+xsize/4;
        }

      //if (xvar >= fMscbInfo.n_variables)
      //xvar = fMscbInfo.n_variables-1;

      xsize += 10;
      int status = mscb_read_range(mscb, fAddr, ivar, xvar, data+iptr, &xsize);
      
      if (0)
	{
	  printf("mscb %d, addr %d, status %d, ivar %d, xvar %d, iptr %d, size %d %d\n", mscb, fAddr, status, ivar, xvar, iptr, size, xsize);
	  for (int i=0; i<xsize; i++)
	    printf("0x%02x ", 0xFF & (*(data+iptr+i)));
	  printf("\n");
	}

      if (status != MSCB_SUCCESS)
        {
          memset(bank, 0, bankSize);
          return status;
        }

      ivar = xvar + 1;
      iptr += xsize;

      size -= xsize;
    }

  copyfunc(bank, bankSize, now, &fMscbInfo, data);

  return SUCCESS;
}

// end
