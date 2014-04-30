/********************************************************************\

  Name:         frontend.c
  Created by:   Stefan Ritt

  Contents:     Example Slow Control Frontend for equipment control
                through EPICS channel access.

  $Log: frontend.c,v $
  Revision 1.6  2000/03/02 21:54:48  midas
  Added comments concerning channel names and possibility to disable functions

  Revision 1.5  1999/12/21 09:37:00  midas
  Use new driver names

  Revision 1.3  1999/10/08 14:04:45  midas
  Set initial channel number to 10

  Revision 1.2  1999/09/22 12:01:09  midas
  Fixed compiler warning

  Revision 1.1  1999/09/22 09:19:25  midas
  Added sources

  Revision 1.3  1998/10/23 08:46:26  midas
  Added #include "null.h"

  Revision 1.2  1998/10/12 12:18:59  midas
  Added Log tag in header


\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midas.h"
#include "generic.h"
#include "epics_ca.h"

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "Epics";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms    */
INT display_period = 000;

/* maximum event size produced by this frontend */
INT max_event_size = 1000;

/* buffer size to hold events */
INT event_buffer_size = 10 * 1000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/*-- Equipment list ------------------------------------------------*/

/* 
The following statement allocates 10 channels for the beamline
control through the epics channel access device driver. The 
EPICS channel names are stored under 
   
  /Equipment/Beamline/Settings/Devices/Beamline

while the channel names as the midas slow control sees them are
under 

  /Equipment/Bemaline/Settings/Names

An example set of channel names is saved in the triumf.odb file
in this directory and can be loaded into the ODB with the odbedit
command

  load triumf.odb

before the frontend is started. The CMD_SET_LABEL statement 
actually defines who determines the label name. If this flag is
set, the CMD_SET_LABEL command in the device driver is disabled,
therefore the label is taken from EPICS, otherwise the label is
taken from MIDAS and set in EPICS.

The same can be done with the demand values. If the command
CMD_SET_DEMAND is disabled, the demand value is always determied
by EPICS.
*/

#define defaultNrChannels 10

/* device driver list */
DEVICE_DRIVER epics_driver[] = {
   {"Epics", epics_ca, defaultNrChannels, 0},       /* , CMD_SET_LABEL }, /* disable CMD_SET_LABEL */
   {""}
};

INT cd_gen(INT cmd, PEQUIPMENT pequipment);
INT cd_gen_read(char *pevent, int);     // copied from generic.h

EQUIPMENT equipment[] = {
   {"Epics",                    /* equipment name */
    {5, 0,                      /* event ID, trigger mask */
     "SYSTEM",                  /* event buffer */
     EQ_SLOW,                   /* equipment type */
     0,                         /* event source */
     "MIDAS",                   /* format */
     TRUE,                      /* enabled */
     511,                       /* Read at every possible opportunity */
     1000,                      /* read every 1 sec */
     0,                         /* stop run after this event limit */
     0,                         /* number of sub events */
     1,                         /* log history every event */
     "", "", ""},               /* extra braces added for fucking Midas 2.0.0 */
    cd_gen_read,                /* readout routine */
    cd_gen,                     /* class driver main routine */
    epics_driver,               /* device driver list */
    NULL,                       /* init string */
    },

   {""}
};

/*-- Dummy routines ------------------------------------------------*/

INT poll_event(INT source[], INT count, BOOL test)
{
   return 1;
};

INT interrupt_configure(INT cmd, INT source[], PTYPE adr)
{
   return 1;
};

/*-- Frontend Init -------------------------------------------------*/
HNDLE hDB = 0;
HNDLE hFE = 0;
HNDLE hDemand = 0;
HNDLE *hODBPV;
INT *ODBPVInd;
DWORD *ODBPVTID;
INT nrChannels;

INT frontend_init()
{
   const int kMaxPath = 100;
   const int kMaxInit = 1000;
   char odbpath[kMaxPath];
   char odbinit[kMaxInit], odbsubinit[kMaxInit];

   int i, size;
   HNDLE hTemp, hKeys;
   INT dbResult;
   KEY key;

   cm_get_experiment_database(&hDB, NULL);

   snprintf(odbpath, kMaxPath, "/Equipment/%s", equipment[0].name);
   if (db_find_key(hDB, 0, odbpath, &hFE) != DB_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot find frontend ODB location");
      return FE_ERR_HW;
   }
   
   snprintf(odbpath, kMaxPath, "Settings/Channels/%s", epics_driver[0].name);
   if (db_create_key(hDB, hFE, odbpath, TID_INT) == DB_SUCCESS) {
      INT temp = defaultNrChannels;
      cm_msg(MERROR, "frontend_init", "number of channels not found. Using default");
      db_set_value(hDB, hFE, odbpath, &temp, sizeof(INT), 1, TID_INT);
   }   
   size = sizeof(INT);
   db_find_key(hDB, hFE, odbpath, &hTemp);
   db_get_data(hDB,hTemp,&nrChannels,&size,TID_INT);
   epics_driver[0].channels = nrChannels;

   db_create_key(hDB, hFE, "Variables/Demand", TID_FLOAT);
   db_find_key(hDB, hFE, "Variables/Demand", &hDemand);
   db_set_num_values(hDB, hDemand, nrChannels);

   dbResult = db_create_key(hDB, hFE, "Settings/Keys", TID_STRING);
   db_find_key(hDB, hFE, "Settings/Keys", &hKeys);
   if (dbResult == DB_SUCCESS) {
      // we created the key, so set the correct default size
      char *str = (char *)calloc(kMaxPath,1);
      db_set_data(hDB, hKeys, str, kMaxPath, 1, TID_STRING);
      free(str);
   }
   db_set_num_values(hDB, hKeys, nrChannels);

   hODBPV = (HNDLE *)calloc(nrChannels, sizeof(HNDLE));
   ODBPVInd = (INT *)calloc(nrChannels, sizeof(INT));
   ODBPVTID = (DWORD *)calloc(nrChannels, sizeof(DWORD));

   /* Fill the hODBPV array with keys from &hTEMP = /equipment/epics/settings/keys[] */
   for (i = 0; i < nrChannels; i++) {     /* Get handles for copying over variables --shouldn't be hardcoded, but oh well */
      hODBPV[i] = 0xdeaddead;     /* Use this to indicate a key not to be used for anything */
      ODBPVTID[i] = 0;    /* type of datum pointed to by key */
      size = kMaxPath;

      /* First get a string containing the name of a key to be tranferred */      
      if (db_get_data_index(hDB, hKeys, odbpath, &size, i, TID_STRING) != DB_SUCCESS) {
	 cm_msg(MERROR, "frontend_init", "Cannot read key %d", i);
	 continue;
      }
      /* Now check that the entry in keys is not empty */
      if (strlen(odbpath) == 0) {       
	 cm_msg(MINFO, "frontend_init", "Key %d is empty", i);
	 continue;
      }
      
      /* Now check if that string is actually a real key */
      if (db_find_key(hDB, 0, odbpath, &hTemp) != DB_SUCCESS) {
	 cm_msg(MERROR, "frontend_init", "Key %d ( %s ) does not exist.", i, odbpath);
	 continue;
      }
      
      hODBPV[i] = hTemp; /* replace 0xdeaddead with valid key locator */
      /* Now check what type of key it is */
      if (db_get_key(hDB, hODBPV[i], &key) == DB_SUCCESS) {
	 ODBPVTID[i] = key.type;
	 ODBPVInd[i] = 0;        /* assume at first that you want the 0th index of that key (e.g. not an array) */
	 char *pch = strchr(odbpath, '[');      /* Check if there is an array index for this key */
	 if (pch != NULL) {
	    sscanf(pch, "[%d]", &ODBPVInd[i]);
	 }
	 if (ODBPVInd[i] >= key.num_values) {
	    cm_msg(MERROR, "frontend_init", "Key %d ( %s ) index out of range. %d values present.",
		   i, odbpath, key.num_values);
	    continue;
	 }
      }
      
      cm_msg(MINFO, "frontend_init", "Key %d ( %s ) set to handle 0x%08x type %s",
	     i, odbpath, hODBPV[i], rpc_tid_name(ODBPVTID[i]));
   }
   return CM_SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
   return CM_SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/


INT frontend_loop()
{
   static DWORD watchdog_time = 0;

   /* slow down frontend not to eat all CPU cycles */
   cm_yield(50);

   if (ss_time() - watchdog_time > 1) {
      float f;
      double d;
      DWORD w;
      INT j;

      int i, size;
      INT dbstat;

      watchdog_time = ss_time();
      for (i=0; i < nrChannels; i++) {
	 if (hODBPV[i] == 0xdeaddead) {
	    continue;
	 }
	 switch (ODBPVTID[i]) {
	 case TID_FLOAT:
	    size = sizeof(float);
	    dbstat = db_get_data_index(hDB, hODBPV[i], &f, &size, ODBPVInd[i], ODBPVTID[i]);	    
	    break;
	 case TID_DOUBLE:
	    size = sizeof(double);
	    dbstat = db_get_data_index(hDB, hODBPV[i], &d, &size, ODBPVInd[i], ODBPVTID[i]);
	    f = d;
	    break;
	 case TID_DWORD:
	    size = sizeof(DWORD);
	    dbstat = db_get_data_index(hDB, hODBPV[i], &w, &size, ODBPVInd[i], ODBPVTID[i]);
	    f = w;
	    break;
	 case TID_INT:
	    size = sizeof(INT);
	    dbstat = db_get_data_index(hDB, hODBPV[i], &j, &size, ODBPVInd[i], ODBPVTID[i]);
	    f = j;
	    break;
	 default:
	    cm_msg(MERROR, "frontend_loop", "Epics readout for type %s not implemented.",
		   rpc_tid_name(ODBPVTID[i]));
	    hODBPV[i] = 0xdeaddead;
	    dbstat = DB_TYPE_MISMATCH;
	    f = 0;
	 }
	 if (dbstat != DB_SUCCESS) {
	    cm_msg(MERROR, "frontend_loop", "Error reading channel %d, err=%d", i, dbstat);
	 }
	 db_set_data_index(hDB, hDemand, &f, sizeof(f), i, TID_FLOAT);
      }
   }
   return CM_SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*------------------------------------------------------------------*/
