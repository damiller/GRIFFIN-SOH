/********************************************************************\

  Name:         epics_ca.c
  Created by:   Stefan Ritt

  Contents:     Epics channel access device driver

  $Log: epics_ca.c,v $
  Revision 1.3  2002/06/06 07:50:12  midas
  Implemented scheme with DF_xxx flags

  Revision 1.2  2000/03/02 21:50:53  midas
  Added set_label command and possibility to disable individual functions

  Revision 1.1  1999/12/20 10:18:19  midas
  Reorganized driver directory structure

  Revision 1.7  1999/09/22 15:27:35  midas
  Removed TABs

  Revision 1.6  1999/09/22 13:01:12  midas
  Figured out that pv_handle is an array of pointers

  Revision 1.5  1999/09/22 12:53:04  midas
  Removed sizeof(chid) by sizeof(struct channel_in_use)

  Revision 1.4  1999/09/22 12:13:42  midas
  Fixed compiler warning

  Revision 1.3  1999/09/22 11:30:48  midas
  Added event driven data readout

  Revision 1.2  1999/09/21 13:48:40  midas
  Fixed compiler warning

  Revision 1.1  1999/08/31 15:16:26  midas
  Added file

  Revision 1.2  1998/10/12 12:18:57  midas
  Added Log tag in header


\********************************************************************/
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>

#include <cadef.h>
#include <epicsVersion.h>

#ifdef _MSC_VER /* collision between varargs and stdarg */
#undef va_arg
#undef va_start
#undef va_end
#endif

/*---- globals -----------------------------------------------------*/

#define CHN_NAME_LENGTH 32 /* length of channel names */

// chid is a pointer to a channel_in_use structure for older versions of the EPICS library
//   and a pointer to an oldChannelNotify class in newer version
#if (!defined VERSION_INT) 
#define MINIMUM_EPICS_VERSION 999999999
#else
#define MINIMUM_EPICS_VERSION VERSION_INT(3,15,0,1)
#endif

#if (defined EPICS_VERSION_INT) && (EPICS_VERSION_INT < MINIMUM_EPICS_VERSION)
typedef channel_in_use chid_struct;
#else
#include <syncGroup.h>
#include <oldAccess.h>
typedef oldChannelNotify chid_struct;
#endif

#include "midas.h"

using std::string;
using std::vector;

typedef struct {
   vector<string> channelNames;
   vector<chid>   pv_handles;
   vector<float>  array;
   INT            num_channels;
   DWORD          flags;
} CA_INFO;

HNDLE hMeasured;
static HNDLE hDB;

void epics_ca_callback(struct event_handler_args args)
{
   CA_INFO *info;
   int     i;
   
   info = (CA_INFO *) args.usr;
   
   /* search channel index */
   for (i=0 ; i<info->num_channels ; i++)
      if (info->pv_handles[i] == args.chid)
	 break;
   
   if (i < info->num_channels)
      info->array[i] = *((float *) args.dbr);
}

/*---- device driver routines --------------------------------------*/

INT epics_ca_init(HNDLE hKey, void **pinfo, INT channels)
{
   int       status, i;
   HNDLE     hTEMP,hBLAH;
   CA_INFO   *info;
   
   /* allocate info structure */
   // info = (CA_INFO *)calloc(1, sizeof(CA_INFO));
   info = new CA_INFO;
   *pinfo = info;
   
   cm_get_experiment_database(&hDB, NULL);
   
   char channel_names[channels][CHN_NAME_LENGTH];
   /* get channel names */
   info->channelNames.resize(channels);

   /* what a pile of horsey poop */
   for (i=0 ; i<channels ; i++) {
      sprintf(channel_names[i], "EPICS:PV%d", i);
   }   

   cm_msg(MINFO, "epics_ca_init", "Initialized with %d channels", channels);
   db_merge_data(hDB, hKey, "Channel Name", 
		 channel_names, CHN_NAME_LENGTH * channels, channels, TID_STRING);

   status = db_find_key(hDB,hKey,"Channel Name",&hTEMP);
   if (status != DB_SUCCESS) {
      return FE_ERR_HW;
   }

   i = CHN_NAME_LENGTH*channels;
   status = db_get_data(hDB,hTEMP,channel_names,&i,TID_STRING);
   if (status != DB_SUCCESS) {
      cm_msg (MERROR,"epics_ca_init","Channel Names couldn't be read");
      return FE_ERR_HW;
   }
   for (i=0 ; i < channels; i++) {
      info->channelNames[i] = channel_names[i];
   }

   /* initialize driver */
   status = ca_task_initialize();
   if (!(status & CA_M_SUCCESS)) {
      cm_msg(MERROR, "epics_ca_init", "unable to initialize");
      return FE_ERR_HW;
   }

   /* allocate arrays */
   info->array.resize(channels);
   info->pv_handles.resize(channels);
   
   /* search channels */
   info->num_channels = channels;
   
   for (i=0 ; i<channels ; i++) {
      status = ca_search(channel_names[i], &info->pv_handles[i]);
      if (!(status & CA_M_SUCCESS)) {
	 cm_msg(MERROR, "epics_ca_init", "cannot find tag %s", channel_names[i]);
      }
   }

   if (ca_pend_io(5.0) == ECA_TIMEOUT) {
      for (i=0; i < channels; i++) {
	 if (ca_state(info->pv_handles[i]) != cs_conn) {
	    cm_msg(MERROR, "epics_ca_init", "cannot connect to %s", 
		   channel_names[i]);
	 }
      }
  }

   /* add change notifications */
   for (i=0 ; i<channels ; i++) {
      /* ignore unconnected channels */
      if (ca_state(info->pv_handles[i]) != cs_conn) {
	 continue;
      }

      status = ca_add_event(DBR_FLOAT, info->pv_handles[i], epics_ca_callback, info, NULL);      
      if (!(status & CA_M_SUCCESS)) {
	 cm_msg(MERROR, "epics_ca_init", "cannot add event to channel %s", channel_names[i]); 
      }
   }

   if (ca_pend_io(5.0) == ECA_TIMEOUT) {
      for (i=0; i < channels; i++) {
	 if (ca_state(info->pv_handles[i]) != cs_conn) {
	    cm_msg(MERROR, "epics_ca_init", "cannot add event to channel %s", 
		   channel_names[i]);
	 }
      }
   }

   /* get a handle to the measured values in the ODB */
   extern HNDLE hFE;
   if (db_find_key(hDB, hFE, "Variables/Measured", &hMeasured) != DB_SUCCESS) {
      cm_msg(MERROR,"epics_ca_init", "cannot find measured variables");
   }

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/
INT epics_ca_exit(CA_INFO *info)
{
   ca_task_exit();

   delete info;

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT epics_ca_set(CA_INFO *info, INT channel, float value)
{
   ca_put(DBR_FLOAT, info->pv_handles[channel], &value);
   
   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT epics_ca_set_all(CA_INFO *info, INT channels, float value)
{
   for (int i=0 ; i<MIN(info->num_channels, channels) ; i++)
      ca_put(DBR_FLOAT, info->pv_handles[i], &value);
   
   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT epics_ca_set_label(CA_INFO *info, INT channels, char *label)
{
   int  status;
   char str[256];
   chid chan_id;

   if (ca_state(info->pv_handles.at(channels)) != cs_conn) {
      cm_msg(MINFO, "epics_ca_set_label", "Skipped setting label for %s since PV is not connected.", info->channelNames.at(channels).c_str());
      return FE_ERR_DRIVER;
   }

   sprintf(str,"%s.DESC", info->channelNames.at(channels).c_str());
   status = ca_search(str, &chan_id);
   
   status = ca_pend_io(2.0);
   if (status != ECA_NORMAL)
      printf("%s not found\n", str);
   
   status = ca_put(ca_field_type(chan_id), chan_id, label);
   ca_pend_event(0.01);
   
   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT epics_ca_get(CA_INFO *info, INT channel, float *pvalue)
{
   ca_pend_event(0.0001);
   *pvalue = info->array[channel];

   db_set_data_index(hDB, hMeasured, &info->array[channel], sizeof(float), channel, TID_FLOAT);

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT epics_ca_get_all(CA_INFO *info, INT channels, float *pvalue)
{
   for (int i=0 ; i<MIN(info->num_channels, channels) ; i++)
      epics_ca_get(info, i, pvalue+i);
   
   return FE_SUCCESS;
}

/*---- device driver entry point -----------------------------------*/

extern "C" {

INT epics_ca(INT cmd, ...)
{
   va_list argptr;
   HNDLE   hKey;
   INT     channel, status;
   DWORD   flags;
   float   value, *pvalue;
   CA_INFO *info;
   char    *label;

   va_start(argptr, cmd);
   status = FE_SUCCESS;

   if (cmd == CMD_INIT) {
      void **pinfo;

      hKey = va_arg(argptr, HNDLE);
      pinfo = va_arg(argptr, void **);
      channel = va_arg(argptr, INT);
      flags = va_arg(argptr, DWORD);
      status = epics_ca_init(hKey, pinfo, channel);
      info = *(CA_INFO**)pinfo;
      info->flags = flags;
   } else {
      info = (CA_INFO*)va_arg(argptr, void *);
      
      switch (cmd) {
      case CMD_INIT:
	 break;
	 
      case CMD_EXIT:
	 status = epics_ca_exit(info);
	 break;
	 
      case CMD_SET:
	 channel = va_arg(argptr, INT);
	 value   = (float) va_arg(argptr, double);
	 status = epics_ca_set(info, channel, value);
	 break;
	 
      case CMD_SET_LABEL:
	 channel = va_arg(argptr, INT);
	 label  = va_arg(argptr, char *);
	 status = epics_ca_set_label(info, channel, label);
	 break;
	 
      case CMD_GET:
	 channel = va_arg(argptr, INT);
	 pvalue  = va_arg(argptr, float*);
	 status = epics_ca_get(info, channel, pvalue);
	 break;
	 
      default:
	 break;
      }
   }
  
   va_end(argptr);
   return status;
}

} // epics_ca defined as extern "C"

/*------------------------------------------------------------------*/
