/********************************************************************\

  Name:         lrs1445a.c
  Created by:   Stefan Ritt

  Contents:     LeCroy LRS 1440 High Voltage Device Driver

  $Id: lrs1445a.c 3255 2006-08-05 03:28:49Z ritt $
  
\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "midas.h"

/*---- globals -----------------------------------------------------*/

typedef struct {
   int address;
} LRS1445A_SETTINGS;

#define LRS1445A_SETTINGS_STR "\
Address = INT : 1\n\
"

typedef struct {
   LRS1445A_SETTINGS settings;
   int num_channels;
   INT(*bd) (INT cmd, ...);     /* bus driver entry function */
   void *bd_info;               /* private info of bus driver */
} LRS1445A_INFO;

/*---- device driver routines --------------------------------------*/

void lrs1445a_switch(LRS1445A_INFO * info)
{
   static INT last_address = -1;
   char str[80];
   INT status;

   if (info->settings.address != last_address) {
      BD_PUTS("MAINFRAME 15\r\n");
      BD_GETS(str, sizeof(str), "MAINFRAME 15\r\n", 1000);
      sprintf(str, "MAINFRAME %02d\r\n", info->settings.address);
      BD_PUTS(str);
      status = BD_GETS(str, sizeof(str), "> ", 2000);
      if (!status) {
         cm_msg(MERROR, "lrs1445a_init",
                "LRS1445A adr %d doesn't respond. Check power and RS232 connection.",
                info->settings.address);
         return;
      }

      last_address = info->settings.address;
   }
}

/*------------------------------------------------------------------*/

INT lrs1445a_init(HNDLE hkey, void **pinfo, INT channels, INT(*bd) (INT cmd, ...))
{
   int status, size;
   char str[1000];
   HNDLE hDB, hkeydd;
   LRS1445A_INFO *info;

   /* allocate info structure */
   info = calloc(1, sizeof(LRS1445A_INFO));
   *pinfo = info;

   cm_get_experiment_database(&hDB, NULL);

   /* create LRS1445A settings record */
   status = db_create_record(hDB, hkey, "DD", LRS1445A_SETTINGS_STR);
   if (status != DB_SUCCESS)
      return FE_ERR_ODB;

   db_find_key(hDB, hkey, "DD", &hkeydd);
   size = sizeof(info->settings);
   db_get_record(hDB, hkeydd, &info->settings, &size, 0);

   info->num_channels = channels;
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
   sprintf(str, "MAINFRAME %02d\r\n", info->settings.address);
   BD_PUTS(str);
   status = BD_GETS(str, sizeof(str), "> ", 3000);
   if (!status) {
      cm_msg(MERROR, "lrs1445a_init",
             "LRS1445A adr %d doesn't respond. Check power and RS232 connection.",
             info->settings.address);
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
             info->settings.address);
      return FE_ERR_HW;
   }
#endif

#if 0
   cm_msg(MINFO, "lrs1445a_init",
	  "LeCroy1440 adr %d: turning HV on!",
	  info->settings.address);

   /* turn on HV main switch */
   BD_PUTS("ON\r");
   BD_GETS(str, sizeof(str), "> ", 5000);
#endif

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT lrs1445a_exit(LRS1445A_INFO * info)
{
   info->bd(CMD_EXIT, info->bd_info);

   free(info);

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

#define MAX_CHAN 256

static time_t next = 0;
static int xactual[MAX_CHAN];
static int xdemand[MAX_CHAN];

/*----------------------------------------------------------------------------*/

INT lrs1445a_read_all(LRS1445A_INFO * info, int demand[MAX_CHAN], int actual[MAX_CHAN])
{
   char str[25600];
   int sign;
   int hvstatus;

   lrs1445a_switch(info);

   printf("read all...");

   /* check if HV enabled */
   BD_PUTS("SHOW STATUS\r");
   BD_GETS(str, sizeof(str), "> ", 3000);

   if (strstr(str, "HV ON")) {
      hvstatus = 1;
      printf("hv is on...");
   }
   else if (strstr(str, "HV OFF")) {
      hvstatus = 0;
      printf("hv is off...");
   }
   else {
      cm_msg(MERROR, "lrs1445a_read_all",
             "LeCroy1440 adr %d: unexpected HV status: %s",
             info->settings.address,
	     str);
      exit(1);
   }

   time_t t0 = time(NULL);

   sprintf(str, "READ (0-15,0-15)\r");
   BD_PUTS(str);
   int rd = BD_GETS(str, sizeof(str), "> ", 30000);

   printf("rd %d...", rd);

   time_t t1 = time(NULL);

   //printf("read time %d, string [%s]\n", t1-t0, str);
   printf("read time %d\n", (int)(t1-t0));

   char* p = strstr(str, "0, 0");
   if (!p)
     return FE_ERR_HW;

   int i;
   for (i=0; i<MAX_CHAN; i++) {
     p = strchr(p, ')');
     if (!p)
       return FE_ERR_HW;

     p += 4;

     sign = 1;
     if (p[7] == '-')
       sign = -1;

     demand[i] = sign*atoi(p+1);

     actual[i] = sign*atoi(p+8);

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
             info->settings.address);

      cm_msg(MINFO, "lrs1445a_init",
	     "LeCroy1440 adr %d: turning HV off!",
	     info->settings.address);

      /* turn off HV main switch */
      BD_PUTS("OFF\r");
      BD_GETS(str, sizeof(str), "> ", 5000);
   }
#endif

   next = t1 + 20;

   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT lrs1445a_set(LRS1445A_INFO * info, INT channel, float value)
{
#if 0
   char str[80];

   lrs1445a_switch(info);

   sprintf(str, "WRITE (%d,%d) %04.0f\r", channel / 16, channel % 16, value);

   BD_PUTS(str);
   BD_GETS(str, sizeof(str), "> ", 1000);
#endif
   return FE_SUCCESS;
}

/*----------------------------------------------------------------------------*/

INT lrs1445a_get(LRS1445A_INFO * info, INT channel, float *pvalue)
{
   double value;
   char *p, str[256];
   int sign = 1;

   lrs1445a_switch(info);

   sprintf(str, "READ (%d,%d)\r", channel / 16, channel % 16);
   BD_PUTS(str);
   BD_GETS(str, sizeof(str), "> ", 1000);
   p = str + 60 + 5;

   if (channel % 16 > 9)
      p++;
   if (channel / 16 > 9)
      p++;

   if (*p == '-') {
     p++;
     sign = -1;
   }

   value = sign*atof(p);

   *pvalue = (float) value;

   //printf("string [%s] [%s] value %f\n", str, p, value);

   return FE_SUCCESS;
}

INT lrs1445a_get_demand(LRS1445A_INFO * info, INT channel, float *pvalue)
{
   double value;
   char *p, str[256];
   int sign = 1;

   lrs1445a_switch(info);

   sprintf(str, "READ (%d,%d)\r", channel / 16, channel % 16);
   BD_PUTS(str);
   BD_GETS(str, sizeof(str), "> ", 1000);
   p = str + 58;

   if (channel % 16 > 9)
      p++;
   if (channel / 16 > 9)
      p++;

   if (*p == '-') {
     p++;
     sign = -1;
   }

   value = sign*atof(p);

   *pvalue = (float) value;

   //printf("string [%s] [%s] value %f\n", str, p, value);

   next = 0;

   return FE_SUCCESS;
}

/*---- device driver entry point -----------------------------------*/

INT lrs1445a(INT cmd, ...)
{
   va_list argptr;
   HNDLE hKey;
   INT channel, status;
   float value, *pvalue;
   void *info, *bd;
   DWORD flags;

   va_start(argptr, cmd);
   status = FE_SUCCESS;

   switch (cmd) {
   case CMD_INIT:
      hKey = va_arg(argptr, HNDLE);
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      flags = va_arg(argptr, DWORD);
      bd = va_arg(argptr, void *);
      status = lrs1445a_init(hKey, info, channel, bd);
      break;

   case CMD_EXIT:
      info = va_arg(argptr, void *);
      status = lrs1445a_exit(info);
      break;

   case CMD_SET:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      value = (float) va_arg(argptr, double);
      status = lrs1445a_set(info, channel, value);
      printf("CMD_SET channel %d, status %d, value %f!\n", channel, status, value);
      xdemand[channel] = value;
      break;

   case CMD_GET:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      pvalue = va_arg(argptr, float *);
      if (0) {
	status = lrs1445a_get(info, channel, pvalue);
      }
      if (1) {
	time_t now = time(NULL);
	if (next==0 || now>next) {
	  status = lrs1445a_read_all(info, xdemand, xactual);
	  if (status != FE_SUCCESS)
	    return status;
	}
	*pvalue = xactual[channel];
	status = FE_SUCCESS;
      }

      if (channel==255)
	ss_sleep(100);

      //if (channel==192)
      //printf("CMD_GET channel %d, status %d, value %f!\n", channel, status, *pvalue);
      break;

   case CMD_GET_CURRENT:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      pvalue = va_arg(argptr, float *);
      //status = lrs1445a_get(info, channel, pvalue);
      *pvalue = 0;
      status = SUCCESS;
      //printf("CMD_GET_CURRENT channel %d, status %d, value %f!\n", channel, status, *pvalue);
      break;

   case CMD_GET_DEMAND:
      //printf("CMD_GET_DEMAND!\n");
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      pvalue = va_arg(argptr, float *);
      if (0) {
	status = lrs1445a_get_demand(info, channel, pvalue);
      }
      if (1) {
	time_t now = time(NULL);
	if (next==0 || now>next) {
	  status = lrs1445a_read_all(info, xdemand, xactual);
	  if (status != FE_SUCCESS)
	    return status;
	}
	*pvalue = xdemand[channel];
	status = FE_SUCCESS;
      }
      break;

   case CMD_GET_VOLTAGE_LIMIT:
      //printf("CMD_GET_VOLTAGE_LIMIT!\n");
      break;

   case CMD_GET_CURRENT_LIMIT:
      //printf("CMD_GET_CURRENT_LIMIT!\n");
      break;

   case CMD_GET_RAMPUP:
      //printf("CMD_GET_RAMPUP!\n");
      break;

   case CMD_GET_RAMPDOWN:
      //printf("CMD_GET_RAMPDOWN!\n");
      break;

   case CMD_GET_TRIP_TIME:
      //printf("CMD_GET_TRIP_TIME!\n");
      break;

   case CMD_SET_VOLTAGE_LIMIT:
      //printf("CMD_SET_VOLTAGE_LIMIT!\n");
      break;

   case CMD_SET_CURRENT_LIMIT:
      //printf("CMD_SET_CURRENT_LIMIT!\n");
      break;

   case CMD_SET_RAMPUP:
      //printf("CMD_SET_RAMPUP!\n");
      break;

   case CMD_SET_RAMPDOWN:
      //printf("CMD_SET_RAMPDOWN!\n");
      break;

   case CMD_SET_TRIP_TIME:
      //printf("CMD_SET_TRIP_TIME!\n");
      break;

   case CMD_GET_LABEL:
   case CMD_SET_LABEL:
      break;

   default:
      cm_msg(MERROR, "lrs1445a device driver", "Received unknown command %d", cmd);
      status = FE_ERR_DRIVER;
      break;
   }

   va_end(argptr);

   return status;
}

/*------------------------------------------------------------------*/
