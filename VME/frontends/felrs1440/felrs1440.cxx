/********************************************************************\

  Name:         sy2527.c
  Created by:   Stefan Ritt

  Contents:     Example Slow Control Frontend program. Defines two
                slow control equipments, one for a HV device and one
                for a multimeter (usually a general purpose PC plug-in
                card with A/D inputs/outputs. As a device driver,
                the "null" driver is used which simulates a device
                without accessing any hardware. The used class drivers
                cd_hv and cd_multi act as a link between the ODB and
                the equipment and contain some functionality like
                ramping etc. To form a fully functional frontend,
                the device driver "null" has to be replaces with
                real device drivers.

  $Id: frontend.c 2779 2005-10-19 13:14:36Z ritt $

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "midas.h"

extern "C"
{
#include "class/hv.h"
#include "device/lrs1445a.h"
#include "bus/null.h"
#include "bus/rs232.h"
}

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "FeLrs1440";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms    */
INT display_period = 000;

/* maximum event size produced by this frontend */
INT max_event_size = 10000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 10000;

/* buffer size to hold events */
INT event_buffer_size = 10 * 10000;

/*-- Equipment list ------------------------------------------------*/

/* device driver list */
DEVICE_DRIVER lrs1445a_driver[] = {
   {"lrs1445a", lrs1445a, 256, rs232, DF_PRIO_DEVICE},
   {""}
};

int read_lrs1440_event(char *pevent, INT off);

#include "../include/e614evid.h"

EQUIPMENT equipment[] = {
#if 0
   {"LRS1440",                  /* equipment name */
    {EVID_HV, 0,                /* event ID, trigger mask */
     "",                        /* event buffer, when empty "", will not send events */
     EQ_SLOW,                   /* equipment type */
     0,                         /* event source */
     "MIDAS",                   /* format */
     TRUE,                      /* enabled */
     RO_RUNNING|RO_STOPPED|RO_PAUSED,  /* read in all states */
     600000,                     /* read every 600 sec */
     0,                         /* stop run after this event limit */
     0,                         /* number of sub events */
     0,                         /* log history every event */
     "", "", ""} ,
    cd_hv_read,                 /* readout routine */
    cd_hv,                      /* class driver main routine */
    lrs1445a_driver,            /* device driver list */
    NULL,                       /* init string */
    },
#endif
    {"LRS1440",               /* equipment name */
     {EVID_HV, 0,             /* event ID, trigger mask */
      "",                     /* event buffer */
      EQ_PERIODIC,            /* equipment type */
      0,                      /* event source */
      "MIDAS",                /* format */
      TRUE,                   /* enabled */
      RO_ALWAYS,              /* when to read this event */
      1000,                   /* poll time in milliseconds */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      0,                      /* whether to log history */
      "", "", "",}
     ,
     read_lrs1440_event,         /* readout routine */
     NULL,
     NULL,
     NULL,       /* bank list */
    },

   {""}
};


/*-- Dummy routines ------------------------------------------------*/

extern "C" INT poll_event(INT source[], INT count, BOOL test)
{
   return 1;
};
extern "C" INT interrupt_configure(INT cmd, INT source[], POINTER_T adr)
{
   return 1;
};

/*-- Frontend Init -------------------------------------------------*/

extern "C" INT frontend_init()
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);
   return CM_SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

extern "C" INT frontend_exit()
{
   return CM_SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

static int i;
extern "C" INT frontend_loop()
{
   i+=1;
   ss_sleep(100);
   return CM_SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

extern "C" INT begin_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

extern "C" INT end_of_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

extern "C" INT pause_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/

extern "C" INT resume_run(INT run_number, char *error)
{
   return CM_SUCCESS;
}

/*------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "midas.h"

/*---- globals -----------------------------------------------------*/

typedef struct {
   INT(*bd) (INT cmd, ...);     /* bus driver entry function */
   void *bd_info;               /* private info of bus driver */
} LRS1445A_INFO;

LRS1445A_INFO *info = NULL;

/*---- device driver routines --------------------------------------*/

void lrs1445a_switch(int addr = -1)
{
   static INT last_address = -1;
   char str[80];
   INT status;

   if (addr != last_address) {
      BD_PUTS("MAINFRAME 15\r\n");
      BD_GETS(str, sizeof(str), "MAINFRAME 15\r\n", 1000);
      sprintf(str, "MAINFRAME %02d\r\n", addr);
      BD_PUTS(str);
      status = BD_GETS(str, sizeof(str), "> ", 2000);
      if (!status) {
         cm_msg(MERROR, "lrs1445a_init",
                "LRS1445A adr %d doesn't respond. Check power and RS232 connection.",
                addr);
         return;
      }

      last_address = addr;
   }
}

/*------------------------------------------------------------------*/

INT lrs1445a_init(HNDLE hDB, HNDLE hkey, INT(*bd) (INT cmd, ...), int addr)
{
   int status;
   char str[1000];

   /* allocate info structure */
   info = (LRS1445A_INFO*) calloc(1, sizeof(LRS1445A_INFO));

   info->bd = bd;

   /* initialize bus driver */
   if (!bd)
      return FE_ERR_ODB;

   status = bd(CMD_INIT, hkey, &info->bd_info);

   if (status != SUCCESS) {
      cm_msg(MERROR, "lrs1445a_init",
             "Cannot access RS232 port. Maybe some other application is using it.");
      return status;
   }

   bd(CMD_DEBUG, FALSE);

   while (1) {
     int u = BD_GETS(str, sizeof(str), "", 2000);
     printf("Flushing the RS232 connection: flush %d bytes\n", u);
     //printf("[%s]\n", str);
     if (u==0)
       break;
   }

   /* check if module is living  */
   sprintf(str, "MAINFRAME %02d\r\n", addr);
   BD_PUTS(str);
   status = BD_GETS(str, sizeof(str), "> ", 3000);
   if (!status) {
      cm_msg(MERROR, "lrs1445a_init",
             "LRS1445A adr %d doesn't respond. Check power and RS232 connection.",
             addr);
      return FE_ERR_HW;
   }

   /* check if HV enabled */
   BD_PUTS("SHOW STATUS\r");
   BD_GETS(str, sizeof(str), "> ", 3000);

   printf("Mainframe status: %s\n", str);

#if 0
   if (strstr(str, "disabled")) {
      cm_msg(MERROR, "lrs1445a_init",
             "LeCroy1440 adr %d: HV disabled by front panel button",
             addr);
      return FE_ERR_HW;
   }
#endif

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT lrs1445a_exit()
{
   info->bd(CMD_EXIT, info->bd_info);

   free(info);

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

#define MAX_CHAN 256

/*----------------------------------------------------------------------------*/

INT lrs1445a_read_hvon(int addr, int *hvon)
{
   char str[25600];

   lrs1445a_switch(addr);

   printf("read hvon...");

   /* check if HV enabled */
   BD_PUTS("SHOW STATUS\r");
   BD_GETS(str, sizeof(str), "> ", 3000);

   if (strstr(str, "HV ON")) {
      printf("hv is on...");
      *hvon = 1;
   }
   else if (strstr(str, "HV OFF")) {
      printf("hv is off...");
      *hvon = 0;
   }
   else {
      cm_msg(MERROR, "lrs1445a_read_all",
             "LeCroy1440 adr %d: unexpected HV status: %s",
             addr,
	     str);
      exit(1);
   }

   printf("\n");

   return FE_SUCCESS;
}

INT lrs1445a_read_channels(int addr, const char* list, int demand[MAX_CHAN], int actual[MAX_CHAN], int timestamp[MAX_CHAN])
{
   char str[25600];

   lrs1445a_switch(addr);

   //RP//printf("read channels %s...", list);

   time_t t0 = time(NULL);

   //sprintf(str, "READ (0-15,0-15)\r");
   sprintf(str, "READ (%s)\r", list);
   BD_PUTS(str);
   int rd = BD_GETS(str, sizeof(str), "> ", 30000);

   //RP//printf("rd %d...", rd);

   time_t t1 = time(NULL);

   //printf("read time %d, string [%s]\n", t1-t0, str);
   //RP//printf("read time %d\n", (int)(t1-t0));

   char* p = strstr(str, "Channel");
   if (!p)
     return FE_ERR_HW;

   while (1) {
     p = strchr(p, '(');
     if (!p)
       break;

     int slot = atoi(p+1);
     assert(slot>=0 && slot<=15);

     p = strchr(p, ',');
     if (!p)
       return FE_ERR_HW;

     int chan = atoi(p+1);
     assert(chan>=0 && chan<=15);

     int i = slot*16 + chan;

     p = strchr(p, ')');
     if (!p)
       return FE_ERR_HW;

     p += 4;

     int sign = 1;
     if (p[7] == '-')
       sign = -1;

     demand[i] = sign*atoi(p+1);

     actual[i] = sign*atoi(p+8);

     timestamp[i] = t1;

     //if (i==192 || i==208)
     //printf("chan %d, sign %d, demand %d, actual %d, str [%s]\n", i, sign, demand[i], actual[i], p);
   }

   int ison = 0;
   for (i=0; i<MAX_CHAN; i++)
     if (fabs(actual[i])>10)
       ison = 1;

#if 0
   if (hvstatus && !ison) {
      cm_msg(MERROR, "lrs1445a_read_all",
             "LeCroy1440 adr %d: HV is turned off by the front panel button",
             addr);

      cm_msg(MINFO, "lrs1445a_init",
	     "LeCroy1440 adr %d: turning HV off!",
	     addr);

      /* turn off HV main switch */
      BD_PUTS("OFF\r");
      BD_GETS(str, sizeof(str), "> ", 5000);
   }
#endif
   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT lrs1445a_set(int addr, INT channel, int value)
{
   char str[80];

   lrs1445a_switch(addr);

   sprintf(str, "WRITE (%d,%d) %04d\r", channel/16, channel%16, value);

   //printf("write: %s\n", str);

   BD_PUTS(str);
   BD_GETS(str, sizeof(str), "> ", 1000);

   //printf("read: %s\n", str);

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

static int xaddress = 0;

int xinit(HNDLE hDB, HNDLE hSet)
{
  int status;
  HNDLE hKey;

  status = db_find_key(hDB, hSet, "Devices/lrs1445a", &hKey);
  assert(status == DB_SUCCESS);

  status = lrs1445a_init(hDB, hKey, rs232, xaddress);
  assert(status == SUCCESS);
  return status;
}

static int xactual[MAX_CHAN];
static int xdemand[MAX_CHAN];
static int xnewdemand[MAX_CHAN];
static int xtime[MAX_CHAN];
static int xhvon = 0;

static int xcontrol = 0;
static int xrampstep[MAX_CHAN];
static float xmaxv[MAX_CHAN];

static int xramping = 0;

static int xperiod = 30;
static time_t xnextRead = 0;

int xwrite(int chan, int value)
{
  if (!xcontrol)
    return SUCCESS;

  printf("Writing channel %d demand voltage %d, old demand %d, old actual %d\n", chan, value, xdemand[chan], xactual[chan]);
  return lrs1445a_set(xaddress, chan, value);
}

static int islot = 0;

int xread()
{
  int updated = 0;
  char str[100];
  
  time_t now = time(NULL);

  if (xnextRead == 0)
    xnextRead = now;

  //printf("xread now %d, next %d, slot %d, xramping %d\n", now, xnextRead, islot, xramping);

  if (now < xnextRead)
    return 0;

  if (islot<0)
    {
      lrs1445a_read_hvon(xaddress, &xhvon);
      updated = 1;

      int ison = 0;
      int isoff = 1;
      for (i=0; i<MAX_CHAN; i++) {
	if (xdemand[i]!=0)
	  isoff = 0;
	if (xdemand[i]!=0 && fabs(xactual[i])>10)
	  ison = 1;
      }

      if (xhvon && !ison && !isoff) {
	cm_msg(MERROR, frontend_name,
	       "LRS1440 address %d: HV tripped or turned off by the front panel button",
	       xaddress);
	
#if 0
	if (xcontrol)
	  {
	    cm_msg(MINFO, "lrs1445a_init",
		   "LeCroy1440 adr %d: turning HV off!",
		   xaddress);
	
	    /* turn off HV main switch */
	    BD_PUTS("OFF\r");
	    BD_GETS(str, sizeof(str), "> ", 5000);

	    xnextRead = 0;
	  }
#endif
      }

      if (xcontrol)
	{
	  // check if we are still ramping voltages
	  for (int i=0; i<MAX_CHAN; i++)
	    if (xnewdemand[i] != xdemand[i])
	      {
		printf("Channel %d is still ramping from %d to %d, now at %d\n", i, xdemand[i], xnewdemand[i], xactual[i]);
		xramping = 5;
	      }
	}
	  
      if (xnextRead)
	xnextRead += xperiod;

      if (xramping)
	{
	  xnextRead = 0;
	  xramping--;
	}
    }
  else
    {
      sprintf(str, "%d,0-15", islot);
      lrs1445a_read_channels(xaddress, str, xdemand, xactual, xtime);
      updated = 1;

      if (xcontrol)
	{
	  for (int k=0; k<16; k++)
	    {
	      int chan = islot*16 + k;

	      if (xdemand[chan] != xnewdemand[chan])
		{
		  if (xrampstep[chan])
		    {
		      if (xdemand[chan] < xnewdemand[chan])
			{
			  int newval = xdemand[chan] + xrampstep[chan];
			  if (newval > xnewdemand[chan])
			    newval = xnewdemand[chan];
			  xwrite(chan, newval);
			}
		      else if (xdemand[chan] > xnewdemand[chan])
			{
			  int newval = xdemand[chan] - xrampstep[chan];
			  if (newval < xnewdemand[chan])
			    newval = xnewdemand[chan];
			  xwrite(chan, newval);
			}
		    }
		  else
		    {
		      xwrite(chan, xnewdemand[chan]);
		    }
		}
	    }
	}
    }

  islot++;
  if (islot>15)
    islot = -1;

  return updated;
}

int xramp(int chan, int value)
{
  if (abs(value) > fabs(xmaxv[chan]))
    {
      printf("demand voltage too high, chan %d, demand %d, max %f\n", chan, value, xmaxv[chan]);

      if (value > 0)
	value = fabs(xmaxv[chan]);
      else if (value < 0)
	value = -fabs(xmaxv[chan]);
    }

  if (value != xnewdemand[chan])
    {
      printf("ramp chan %d to %d\n", chan, value);
      xnewdemand[chan] = value;
    }
  return SUCCESS;
}

/*------------------------------------------------------------------*/

int write_data_float(HNDLE hDB, HNDLE hVar, char* keyname, int num, const float data[])
{
   int status = db_set_value(hDB, hVar, keyname, data, num*sizeof(float), num, TID_FLOAT);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}

int write_data_int(HNDLE hDB, HNDLE hVar, char* keyname, int num, const int data[])
{
   int status = db_set_value(hDB, hVar, keyname, data, num*sizeof(int), num, TID_INT);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}

int write_data_string(HNDLE hDB, HNDLE hVar, char* keyname, int num, char* data[])
{
   const int len = NAME_LENGTH;
   char val[num*len];

   for (int i=0; i<num; i++)
     strlcpy(val+i*len, data[i], len);

   int status = db_set_value(hDB, hVar, keyname, val, num*len, num, TID_STRING);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}

int write_data_string(HNDLE hDB, HNDLE hVar, char* keyname, const char* data, int max_length)
{
   int status = db_set_value(hDB, hVar, keyname, data, max_length, 1, TID_STRING);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}

#include <stdint.h>
#include "utils.cxx"

/*------------------------------------------------------------------*/

#define MAX_CHAN 256

int   odb_mainSwitch = 0;
int   odb_switch[MAX_CHAN];
float odb_demand[MAX_CHAN];
float odb_adjust[MAX_CHAN];
char  odb_names[MAX_CHAN][NAME_LENGTH];

int   hw_hvon[1];
int   hw_time[MAX_CHAN];
float hw_demand[MAX_CHAN];
float hw_measured[MAX_CHAN];

int   mf_status[1];        // status of mainframe
int   st_status[MAX_CHAN]; // status of each channel
char* st_color[MAX_CHAN];  // color of each channel

#define ST_OK                  0
#define ST_RAMPING           100
#define ST_UNKNOWN           200
#define ST_BADSTATE          300
#define ST_DEMAND_MISMATCH   400
#define ST_MEASURED_MISMATCH 500

HNDLE hDB;
HNDLE hVar;
HNDLE hSet;
HNDLE hStatus;

static void update_settings(INT a, INT b, void *c)
{
  int status;
  printf("update_settings!\n");

  xaddress = odbReadInt("/Equipment/LRS1440/Settings/Address", 0, 1);

  odb_mainSwitch = odbReadInt("/Equipment/LRS1440/Settings/MainSwitch", 0, 0);
  printf("main switch %d\n", odb_mainSwitch);

  xperiod = odbReadInt("/Equipment/LRS1440/Settings/ReadPeriod", 0, 30);

  xcontrol = odbReadInt("/Equipment/LRS1440/Settings/EnableControl", 0, 0);

  int rampstep = odbReadInt("/Equipment/LRS1440/Settings/RampStep", 0, 100);

  float maxv   = odbReadFloat("/Equipment/LRS1440/Settings/MaxVoltage", 0, 0);

  odbReadString("/Equipment/LRS1440/Settings/Names", MAX_CHAN-1, "", NAME_LENGTH);

  int size;
  
  for (int i=0; i<MAX_CHAN; i++)
    {
      const char* name = odbReadString("/Equipment/LRS1440/Settings/Names", i, NULL, NAME_LENGTH);

      strlcpy(odb_names[i], name, NAME_LENGTH);

      xrampstep[i] = rampstep;

      xmaxv[i] = maxv;

      char group[256];
      strlcpy(group, name, sizeof(group));
      char *ss = strchr(group, '%');
      if (ss)
	{
	  *ss = 0;
	  char str[256];

	  sprintf(str, "/Equipment/LRS1440/Settings/RampStep %s", group);
	  xrampstep[i] = odbReadInt(str, 0, 0);

	  sprintf(str, "/Equipment/LRS1440/Settings/MaxVoltage %s", group);
	  xmaxv[i] = odbReadFloat(str, 0, 0);
	}

      //printf("name [%s], group [%s], rampstep %d, maxv %f\n", name, group, xrampstep[i], xmaxv[i]);
    }

  write_data_int(  hDB, hStatus, "RampStep",   MAX_CHAN, xrampstep);
  write_data_float(hDB, hStatus, "MaxVoltage", MAX_CHAN, xmaxv);

  size = MAX_CHAN*sizeof(int);
  status = db_get_value(hDB, hSet, "outputSwitch", odb_switch, &size, TID_INT, TRUE);
  assert(status == DB_SUCCESS);

  size = MAX_CHAN*sizeof(float);
  status = db_get_value(hDB, hSet, "outputVoltage", odb_demand, &size, TID_FLOAT, TRUE);
  assert(status == DB_SUCCESS);

  size = MAX_CHAN*sizeof(float);
  status = db_get_value(hDB, hSet, "measuredAdjust", odb_adjust, &size, TID_FLOAT, TRUE);
  assert(status == DB_SUCCESS);

  if (xcontrol)
    {
      for (int i=0; i<MAX_CHAN; i++)
	{
	  if (odb_switch[i])
	    xramp(i, (int)odb_demand[i]);
	  else
	    xramp(i, 0);
	}
    }

  xnextRead = 0;
}

static void open_hotlink(HNDLE hDB, HNDLE hSet)
{
  static bool once = false;
  if (once)
    return;
  once = true;

  int status = db_open_record(hDB, hSet, NULL, 0, MODE_READ, update_settings, NULL);
  assert(status == DB_SUCCESS);
}

int read_lrs1440_event(char *pevent, INT off)
{
  time_t now;
  time(&now);

  //printf("read_lrs1440_event %d\n", now);

  static int once = 1;
  if (once)
    {
      once = 0;

      hVar = odbGetHandle(hDB, 0, "/Equipment/LRS1440/Variables", TID_KEY);
      hSet = odbGetHandle(hDB, 0, "/Equipment/LRS1440/Settings", TID_KEY);
      hStatus = odbGetHandle(hDB, 0, "/Equipment/LRS1440/Status", TID_KEY);

      int size;
      int status;

      size = MAX_CHAN*sizeof(float);
      status = db_get_value(hDB, hVar, "Demand", hw_demand, &size, TID_FLOAT, TRUE);
      assert(status == DB_SUCCESS);

      size = MAX_CHAN*sizeof(float);
      status = db_get_value(hDB, hVar, "Measured", hw_measured, &size, TID_FLOAT, TRUE);
      assert(status == DB_SUCCESS);

      mf_status[0] = ST_UNKNOWN;
      for (int i=0; i<MAX_CHAN; i++)
	{
	  st_status[i] = ST_UNKNOWN;
	  st_color[i] = "gray";
	}

      // lock
      for (int i=0; i<MAX_CHAN; i++)
	{
	  xdemand[i] = (int)hw_demand[i];
	  xactual[i] = (int)hw_measured[i];
	  xtime[i] = 0;
	}
      // unlock

      update_settings(0,0,0);

      xinit(hDB, hSet);

      open_hotlink(hDB, hSet);
    }

  if (xcontrol && odb_mainSwitch > 0 && xhvon == 0)
    {
      int status;
      char str[1000];
      
      cm_msg(MINFO, "read_lrs1440_event", "LeCroy1440: turning HV on!");
      
      /* turn on HV main switch */
      BD_PUTS("ON\r");
      BD_GETS(str, sizeof(str), "> ", 5000);
      
      printf("Mainframe reply: %s\n", str);
      xramping = 5;
      islot = -1;
    }

  if (xcontrol && odb_mainSwitch == 0 && xhvon != 0)
    {
      int status;
      char str[1000];
      
      cm_msg(MINFO, "read_lrs1440_event", "LeCroy1440: turning HV off!");
      
      /* turn on HV main switch */
      BD_PUTS("OFF\r");
      BD_GETS(str, sizeof(str), "> ", 5000);
      
      printf("Mainframe reply: %s\n", str);
      xramping = 5;
      islot = -1;
    }


  int update_var = xread();

  // lock
  hw_hvon[0] = xhvon;
  for (int i=0; i<MAX_CHAN; i++)
    {
      hw_demand[i]   = xdemand[i];
      hw_measured[i] = xactual[i] + odb_adjust[i];
      if (i==197 || i==218) hw_measured[i] = 0;
      hw_time[i]     = xtime[i];
    }
  // unlock

  static int lastReadTime = 0;
  int maxReadTime = hw_time[0];
  for (int i=0; i<MAX_CHAN; i++)
    if (hw_time[i] > maxReadTime)
      maxReadTime = hw_time[i];

  if (maxReadTime > lastReadTime)
    {
      lastReadTime = maxReadTime;
      update_var = 1;
    }

  int update_st = 0;

  int new_mf_status = 0;

  char mf_color[256];
  char mf_message[2560];
  std::string smf_message;

  sprintf(mf_color, "%s", "");
  sprintf(mf_message, "%s", "Ok");
  
  if (xramping)
    {
      new_mf_status = ST_RAMPING;
      smf_message += "ramping voltages | ";
    }

  if (odb_mainSwitch != hw_hvon[0])
    {
      new_mf_status = ST_BADSTATE;
      smf_message += "bad main switch state | ";
    }

  for (int i=0; i<MAX_CHAN; i++)
    {
      int newst = st_status[i];
      char* newcol = st_color[i];

      if (fabs(hw_time[i] - now) > 2*xperiod) {
	newst = ST_UNKNOWN;
	newcol = "gray";
      } else if (hw_hvon[0] && odb_switch[i] && (odb_demand[i] != hw_demand[i])) {
	newst = ST_DEMAND_MISMATCH;
	newcol = "#FFFFBB";
      } else if (hw_hvon[0] && (!odb_switch[i]) && (hw_demand[i]!=0)) {
	newst = ST_DEMAND_MISMATCH;
	newcol = "#FFFFBB";
      } else if (fabs(hw_demand[i] - hw_measured[i]) > 10.0) {
	newst = ST_MEASURED_MISMATCH;
	newcol = "#FFBBBB";
      } else {
	newst = ST_OK;
	newcol = "";
      }
      
      if (newst != ST_OK && newst != ST_UNKNOWN)
	{
	  char str[256];
	  sprintf(str, "%s status %d | ", odb_names[i], newst);
	  smf_message += str;
	}

      if (newst != st_status[i] || strcmp(newcol, st_color[i])!=0)
	{
	  st_status[i] = newst;
	  st_color[i] = newcol;
	  update_st = 1;
	}

      if (newst > new_mf_status)
	new_mf_status = newst;
    }

  if (new_mf_status != mf_status[0])
    {
      mf_status[0] = new_mf_status;
      update_st = 1;
      printf("new mainframe status %d\n", new_mf_status);
    }

  if (update_var)
    {
      //RP//printf("writing ODB Variables\n");

      write_data_float(hDB, hVar, "Demand", MAX_CHAN, hw_demand);
      write_data_float(hDB, hVar, "Measured", MAX_CHAN, hw_measured);
    }

  if (update_var || update_st)
    {
      //RP//printf("writing ODB Status\n");

      switch (mf_status[0])
	{
	case ST_UNKNOWN:
	  strlcpy(mf_color, "gray", sizeof(mf_color));
	  break;
	case ST_OK:
	  strlcpy(mf_color, "#BBFFBB", sizeof(mf_color));
	  break;
	default:
	  strlcpy(mf_color, "#FFBBBB", sizeof(mf_color));
	  break;
	}

      strlcpy(mf_message, smf_message.c_str(), sizeof(mf_message));

      write_data_int(hDB, hStatus, "Status", 1, mf_status);
      write_data_int(hDB, hStatus, "HVON", 1, hw_hvon);
      write_data_int(hDB, hStatus, "outputTimestamp", MAX_CHAN, hw_time);
      write_data_int(hDB, hStatus, "outputStatus", MAX_CHAN, st_status);
      write_data_string(hDB, hStatus, "outputColor", MAX_CHAN, st_color);
      write_data_string(hDB, hStatus, "statusColor", mf_color, 32);
      write_data_string(hDB, hStatus, "statusMessage", mf_message, 1024);
    }
 
  return 0;
}

// end
