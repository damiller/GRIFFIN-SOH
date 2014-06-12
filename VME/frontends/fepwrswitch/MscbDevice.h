//
// MscbDevice.h
//

#ifndef INCLUDE_MSCB_DEVICE_H
#define INCLUDE_MSCB_DEVICE_H

#include <stdint.h>

#include "mscb.h"

typedef void (*MscbDeviceBankCopy)(void*bank, int bankSize, uint32_t time, const MSCB_INFO* mscb_info, const char* data);

class MscbDevice
{
 public:
  char* fName;     /// device name
  bool  fTrace;    /// trace all transactions with the device
  int   fAddr;     /// address on the MSCB bus
  int   fDataSize; /// data size
  MSCB_INFO fMscbInfo; /// MSCB_INFO structure

 public:
  MscbDevice(const char* name, int addr); /// ctor
  ~MscbDevice(); /// dtor

  int Init(int mscb, const char* nodeName, int numVariables, int svnRevisionMin, int svnRevisionMax, int dataSize); /// returns MSCB_SUCCESS
  int ShowVariables(int mscb);

  int ReadBank(int mscb, void* bank, int bankSize, MscbDeviceBankCopy copyFunc);
  int ReadBankSlow(int mscb, void* bank, int bankSize);

  int WriteFloat(int mscb, int ivar, float value);
  int WriteWord(int mscb, int ivar, uint16_t value);
  int WriteByte(int mscb, int ivar, unsigned char value);

  int ReadFloat(int mscb, int ivar, float *value);
  int ReadWord(int mscb, int ivar, uint16_t *value);
  int ReadByte(int mscb, int ivar, unsigned char *value);

  float ReadFloat(int mscb, int ivar) { float v; ReadFloat(mscb, ivar, &v); return v; };
  int   ReadWord(int mscb, int ivar)  { unsigned char v; ReadByte(mscb, ivar, &v); return v; };
  int   ReadByte(int mscb, int ivar)  { unsigned char v; ReadByte(mscb, ivar, &v); return v; };

};

static inline void u8be2local(void*dst,const void* src)
{
   char*d = (char*)dst;
   const char*s = (const char*)src;
   *d = *s;
}

static inline void u16be2local(void*dst,const void* src)
{
   uint16_t*d = (uint16_t*)dst;
   const unsigned char*s = (const unsigned char*)src;
   (*d) = s[1] | s[0]<<8;
}

static inline void u32be2local(void*dst,const void* src)
{
   uint32_t*d = (uint32_t*)dst;
   const unsigned char*s = (const unsigned char*)src;
   (*d) = s[3] | s[2]<<8 | s[1]<<16 | s[0]<<24;
}

#endif
// end
