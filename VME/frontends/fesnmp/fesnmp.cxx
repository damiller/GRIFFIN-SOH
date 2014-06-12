/********************************************************************\

  Name:         fesnmp.cxx
  Created by:   K.Olchanski

  Contents:     Generic frontend for snmpwalk

  $Id$

\********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <vector>
#include <map>

#include "midas.h"

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef FE_NAME
#define FE_NAME "fesnmp"
#endif

#ifndef EQ_NAME
#define EQ_NAME "Snmp"
#endif

#ifndef EQ_EVID
#define EQ_EVID 1
#endif


/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
   const char *frontend_name = FE_NAME;
/* The frontend file name, don't change it */
   const char *frontend_file_name = __FILE__;
/* frontend_loop is called periodically if this variable is TRUE    */
   BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms */
   INT display_period = 0;

/* maximum event size produced by this frontend */
   INT max_event_size = 200*1024;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
   INT max_event_size_frag = 1024*1024;

/* buffer size to hold events */
   INT event_buffer_size = 1*1024*1024;

  extern HNDLE hDB;

   char eq_name[256];

/*-- Function declarations -----------------------------------------*/
  INT frontend_init();
  INT frontend_exit();
  INT begin_of_run(INT run_number, char *error);
  INT end_of_run(INT run_number, char *error);
  INT pause_run(INT run_number, char *error);
  INT resume_run(INT run_number, char *error);
  INT frontend_loop();
  
  INT read_snmp_event(char *pevent, INT off);

/*-- Equipment list ------------------------------------------------*/
  
  EQUIPMENT equipment[] = {

    {
#ifdef USE_EB
       EQ_NAME "%02d",          /* equipment name */
#else
       EQ_NAME,          /* equipment name */
#endif
       {
          EQ_EVID, (1<<EQ_EVID),  /* event ID, trigger mask */
          "SYSTEM",               /* event buffer */
#ifdef USE_EB
          EQ_PERIODIC|EQ_EB,      /* equipment type */
#else
          EQ_PERIODIC,            /* equipment type */
#endif
          0,                      /* event source */
          "MIDAS",                /* format */
          TRUE,                   /* enabled */
          RO_ALWAYS,              /* when to read this event */
          10,                     /* poll time in milliseconds */
          0,                      /* stop run after this event limit */
          0,                      /* number of sub events */
          1,                      /* period for logging history, seconds */
          "", "", "",
       }
       ,
       read_snmp_event,         /* readout routine */
       NULL,
       NULL,
       NULL,       /* bank list */
    }
    ,
    {""}
  };
  
#ifdef __cplusplus
}
#endif
/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

#include "utils.cxx"

/*-- Global variables ----------------------------------------------*/

extern "C" int frontend_index;

static std::string gSnmpwalkCommand;
static int gReadPeriod = 10;

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
  return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error)
{
  return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/
INT pause_run(INT run_number, char *error)
{
  return SUCCESS;
}

/*-- Resume Run ----------------------------------------------------*/
INT resume_run(INT run_number, char *error)
{
  return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop()
{
  ss_sleep(10);
  return SUCCESS;
}

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Interrupt configuration ---------------------------------------*/
extern "C" INT interrupt_configure(INT cmd, INT source, PTYPE adr)
{
  assert(!"interrupt_configure() is not implemented");
  return 0;
}

void Replace(std::string&x, const char* replace, const char* with)
{
  int i=x.find(replace);
  if (i>=0)
    x.replace(i, strlen(replace), with);
}

#if 0
int write_data_float(HNDLE hDB, HNDLE hVar, const char* keyname, int num, const std::vector<float>& data)
{
   float val[num];
   
   if ((int)data.size() != num)
      printf("key %s, num %d, data.size: %d\n", keyname, num, (int)data.size());

   assert((int)data.size() == num);
      
   for (int i=0; i<num; i++)
      val[i] = data[i];

   int status = db_set_value(hDB, hVar, keyname, val, num*sizeof(float), num, TID_FLOAT);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}

int write_data_int(HNDLE hDB, HNDLE hVar, const char* keyname, int num, const std::vector<int>& data)
{
   int val[num];

   if ((int)data.size() != num)
      printf("key %s, num %d, data.size: %d\n", keyname, num, (int)data.size());

   assert((int)data.size() == num);
      
   for (int i=0; i<num; i++)
      val[i] = data[i];

   int status = db_set_value(hDB, hVar, keyname, val, num*sizeof(int), num, TID_INT);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}

int write_data_string(HNDLE hDB, HNDLE hVar, const char* keyname, int num, const std::vector<std::string>& data, int length)
{
   char val[length*num];

   if ((int)data.size() != num)
      printf("key %s, num %d, data.size: %d\n", keyname, num, (int)data.size());

   assert((int)data.size() == num);
      
   for (int i=0; i<num; i++)
      strlcpy(val+length*i, data[i].c_str(), length);

   int status = db_set_value(hDB, hVar, keyname, val, num*length, num, TID_STRING);
   assert(status == DB_SUCCESS);
   
   return SUCCESS;
}
#endif

#if 0
int set_snmp_float(const char* name, int index, float value)
{
   if (!gEnableControl)
      return SUCCESS;

   char str[1024];
   char s[1024];

   if (index<0)
      s[0] = 0;
   else
      sprintf(s, ".u%d", index);
   
   sprintf(str, "snmpset -v 2c -M +%s -m +WIENER-CRATE-MIB -c guru %s %s%s F %f", WIENER_MIB_DIR, gWienerDev.c_str(), name, s, value);
   
   printf("Set wiener float: %s\n", str);
   
   system(str);

   return SUCCESS;
}

int set_snmp_int(const char* name, int index, int value)
{
   if (!gEnableControl)
      return SUCCESS;

   char str[1024];
   char s[1024];

   if (index<0)
      s[0] = 0;
   else
      sprintf(s, ".u%d", index);

   sprintf(str, "snmpset -v 2c -M +%s -m +WIENER-CRATE-MIB -c guru %s %s%s i %d", WIENER_MIB_DIR, gWienerDev.c_str(), name, s, value);

   printf("Set wiener integer: %s\n", str);

   system(str);

   return SUCCESS;
}
#endif

extern HNDLE hDB;
static HNDLE hVar = 0;
static HNDLE hSet = 0;
static HNDLE hRdb = 0;

static int gCommError = 0;

static time_t gNextRead = 0;
static int    gNextUpdate = 0;
static int    gFastRead = 0;

struct HistorySettings {
   char readbackName[NAME_LENGTH];
   bool enabled;
   time_t period_sec;
   char variableName[NAME_LENGTH];

   HistorySettings() // ctor
   {
      readbackName[0] = 0;
      enabled = false;
      period_sec = 0;
      variableName[0] = 0;
   }
};

typedef std::map<std::string,HistorySettings> HistorySettingsMap;

static HistorySettingsMap gHsMap;

void ClearHsMap()
{
   gHsMap.clear();
}

void InitOdbHsMap()
{
   char str[1024];

   sprintf(str, "/Equipment/%s/Settings/History/***example***", eq_name);
   
   odbReadString(str, 0, "variableIsEnabled(y/n),variableUpdatePeriod(sec),variableName", 200);
}

static const HistorySettings& ReadHsMapEntry(const char* readbackName)
{
   HistorySettings &hs = gHsMap[readbackName];
   //printf("readback name [%s], hs [%s]\n", readbackName, hs.readbackName);

   if (hs.readbackName[0])
      return hs;

   char str[1024];
   char val[1024];

   sprintf(str, "/Equipment/%s/Settings/History/%s", eq_name, readbackName);
   sprintf(val, "n,%d,%s", 1, readbackName);
   
   const char* s = odbReadString(str, 0, val, 200);

   //printf("readback [%s], hs [%s]\n", readbackName, s);

   strlcpy(hs.readbackName, readbackName, sizeof(hs.readbackName));
   hs.period_sec = gReadPeriod;
   strlcpy(hs.variableName, readbackName, sizeof(hs.variableName));

   if (strlen(s) > 1) {
      const char* syn = s;
      if (syn[0] == 'y') {
         hs.enabled = true;

         const char* speriod = strchr(syn, ',');
         //printf("speriod [%s]\n", speriod);
         if (speriod && speriod[1]) {
            speriod += 1; // skip the comma
            int period = atoi(speriod);
            if (period < gReadPeriod)
               period = gReadPeriod;
            //printf("period %d\n", period);
            hs.period_sec = period;

            const char* sname = strchr(speriod, ',');
            if (sname && sname[1]) {
               sname += 1; // skip the comma
               //printf("variable name [%s]\n", sname);
               strlcpy(hs.variableName, sname, sizeof(hs.variableName));
            }
         }
      }
   }

   //exit(0);

   return hs;
}

static void handle_hotlink(INT a, INT b, void *c)
{
   printf("handle_hotlink!\n");
   gNextRead   = time(NULL) + 1;
   gNextUpdate = 1;
}

static int update_settings()
{
   char str[1024];
   
   printf("update_settings!\n");
   
   sprintf(str, "/Equipment/%s/Settings/SnmpwalkCommand", eq_name);
   
   gSnmpwalkCommand = odbReadString(str, 0, "", 200);
   
   sprintf(str, "/Equipment/%s/Settings/ReadPeriod", eq_name);
   
   gReadPeriod = odbReadInt(str, 0, 10);

   if (gSnmpwalkCommand.length() <= 2) {
      cm_msg(MERROR, frontend_name, "Please specify the snmpwalk command in frontend Settings!");
      return -1;
   }
   
   if (gReadPeriod < 10 || gReadPeriod > 24*60*60) {
      cm_msg(MERROR, frontend_name, "Please specify valid readout period in frontend Settings!");
      return -1;
   }

   ClearHsMap();
   InitOdbHsMap();
   
   gNextRead = time(NULL);
   //gFastRead = 10;

   return 0;
}

static void open_hotlink(HNDLE hDB, HNDLE hSet)
{
  static bool once = false;
  if (once)
    return;
  once = true;

  update_settings();

  int status = db_open_record(hDB, hSet, NULL, 0, MODE_READ, handle_hotlink, NULL);
  assert(status == DB_SUCCESS);
}

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
  setbuf(stdout,NULL);
  setbuf(stderr,NULL);

#ifdef USE_EB
  if (frontend_index < 1)
    {
      cm_msg(MERROR, frontend_name, "frontend_init(): Frontend index %d is not valid, please start with \'-i 1\'", frontend_index);
      return !SUCCESS;
    }

  sprintf(eq_name, "%s%02d", EQ_NAME, frontend_index);
#else
  sprintf(eq_name, "%s", EQ_NAME);
#endif

  int status;
  char str[1024];

  sprintf(str, "/Equipment/%s/Settings", eq_name);

  status = odbMkdir(str, &hSet);
  if (status != 0)
      return FE_ERR_ODB;

  sprintf(str, "/Equipment/%s/Variables", eq_name);

  status = odbMkdir(str, &hVar);
  if (status != 0)
      return FE_ERR_ODB;

  sprintf(str, "/Equipment/%s/Readback", eq_name);

  status = odbMkdir(str, &hRdb);
  if (status != 0)
      return FE_ERR_ODB;

  status = update_settings();
  if (status != 0)
      return FE_ERR_ODB;

  open_hotlink(hDB, hSet);

  set_equipment_status(eq_name, "Init Ok", "#00FF00");

  return SUCCESS;
}

int read_snmp_event(char *pevent, INT off)
{
  time_t now;
  time(&now);


  if (now < gNextRead)
     return 0;

  if (gFastRead > 0)
     {
	gFastRead--;
	gNextRead = now + 1;
     }
  else
     gNextRead += gReadPeriod; // ODB settings read period

  if (gNextRead < now)
     gNextRead = now + 10; // ODB settings read period

  if (gNextUpdate) {
     gNextUpdate = 0;
     update_settings();
     return 0;
  }

  //
  // Note: please copy WIENER-CRATE-MIB.txt to /usr/share/snmp/mibs/
  //
  // Control commands:
  //
  // snmpset -v 2c -m +WIENER-CRATE-MIB -c guru daqtmp1 sysMainSwitch.0 i 1
  // snmpset -v 2c -m +WIENER-CRATE-MIB -c guru daqtmp1 outputSwitch.u100 i 1
  // snmpset -v 2c -m +WIENER-CRATE-MIB -c guru daqtmp1 outputVoltage.u100 F 10
  //

  char str[1024];

  assert(gSnmpwalkCommand.length() > 0);

  sprintf(str, "%s 2>&1", gSnmpwalkCommand.c_str());

  printf("Snmpwalk command: %s\n", str);

  FILE *fp = popen(str, "r");
  if (fp == NULL)
    {
      ss_sleep(200);
      return 0;
    }

  while (1)
    {
      char *s = fgets(str, sizeof(str), fp);
      if (!s)
         break;

      //printf("wiener: %s\n", s);

      char name[1024];

      s = strstr(str, "No Response from");
      if (s)
         {
            printf("No response from : %s", str);
            if (gCommError == 0) {
               cm_msg(MERROR, frontend_name, "read_snmp_event: No response from device: %s", str);
               gCommError = 1;
            }
            continue;
         }

      s = strstr(str, "-MIB::");
      if (s == NULL)
         {
            gCommError = 1;
            printf("unknown response (no -MIB::) : %s", str);
            continue;
         }

      strlcpy(name, s+6, sizeof(name));

      char* q = strstr(name, " = ");
      if (q == NULL)
         {
            printf("unknown response (no \'=\'): %s", str);
            continue;
         }
      
      *q = 0;

      //printf("name [%s]\n", name);

      //continue;

      if (0) 
         { 
            std::string x = name; 
            Replace(x, "tempHumidSensor", "ths"); 
            strlcpy(name, x.c_str(), sizeof(name)); 
         } 
      
      if (strlen(name) > NAME_LENGTH-1)
         {
            int len = strlen(name);
            int extra = len - NAME_LENGTH + 1;
            assert(extra > 0);
            assert(extra < 1000);
            memmove(name, name+extra, NAME_LENGTH);
            name[NAME_LENGTH] = 0;
         }
      
      if (strlen(name) > NAME_LENGTH-1)
         {
            cm_msg(MERROR, frontend_name, "read_snmp_event: Variable name \'%s\' is too long %d, limit %d, please rename it in %s.", name, strlen(name), NAME_LENGTH-1, __FILE__);
            continue;
         }
      
      if (gCommError != 0) {
         cm_msg(MINFO, frontend_name, "Device communication is okey now");
         gCommError = 0;
      }

      const HistorySettings& hs = ReadHsMapEntry(name);
      
      if ((s = strstr(str, "No more variables")) != NULL)
         {
            continue;
         }
      else if ((s = strstr(str, "INTEGER:")) != NULL)
         {
            s += 8;
            while (*s != 0)
               {
                  if (isdigit(*s))
                     break;
                  if (*s == '-')
                     break;
                  if (*s == '+')
                     break;
                  s++;
               }
            
            int val = atoi(s);
            //printf("%s = int value %d from %s", name, val, str);
            db_set_value(hDB, hRdb, name, &val, sizeof(val), 1, TID_INT);
            if (hs.enabled)
               db_set_value(hDB, hVar, hs.variableName, &val, sizeof(val), 1, TID_INT);
         }
      else if ((s = strstr(str, "Float:")) != NULL)
         {
            float val = atof(s + 6);
            //printf("%s = float value %f\n", name, val);
            db_set_value(hDB, hRdb, name, &val, sizeof(val), 1, TID_FLOAT);
            if (hs.enabled)
               db_set_value(hDB, hVar, hs.variableName, &val, sizeof(val), 1, TID_FLOAT);
         }
      else if ((s = strstr(str, "BITS:")) != NULL)
         {
            uint32_t val = 0;
            char* ss = s+5;
            
            //printf("bits %s\n", ss);
            
            int ishift = 0;
            
            while (*ss)
               {
                  while (isspace(*ss))
                     ss++;
                  
                  int xval = 0;
                  
                  if (isdigit(*ss)) {
                     xval = (*ss - '0');
                  } else if ((toupper(*ss) >= 'A') && (toupper(*ss) <= 'F')) {
                     xval = 10 + (toupper(*ss) - 'A');
                  } else {
                     break;
                  }
                  
                  // bits go in reverse order
                  
                  int ival = 0;
                  
                  if (xval&1)
                     ival |= 8;
                  
                  if (xval&2)
                     ival |= 4;
                  
                  if (xval&4)
                     ival |= 2;
                  
                  if (xval&8)
                     ival |= 1;
                  
                  val |= (ival<<ishift);
                  
                  ishift += 4;
                  
                  ss++;
               }
            
            if (1)
               {
                  char *xss;
                  xss = strchr(ss, '\n');
                  if (xss)
                     *xss = 0;
                  xss = strchr(ss, '\r');
                  if (xss)
                     *xss = 0;
               }
            
            char* text = ss;
            
            //printf("%s = bit value 0x%08x from [%s], text [%s]\n", name, val, str, text);
            
            db_set_value(hDB, hRdb, name, &val, sizeof(val), 1, TID_DWORD);
            if (hs.enabled)
               db_set_value(hDB, hVar, hs.variableName, &val, sizeof(val), 1, TID_DWORD);
         }
      else if ((s = strstr(str, "STRING:")) != NULL)
         {
            char *ss =  (s + 8);
            while (isspace(*ss))
               ss++;
            if (ss[strlen(ss)-1]=='\n')
               ss[strlen(ss)-1]=0;
            
            //printf("%s = string value [%s]\n", name, ss);
            db_set_value(hDB, hRdb, name, ss, strlen(ss)+1, 1, TID_STRING);
            if (0 && hs.enabled) // history cannot record strings
               db_set_value(hDB, hVar, hs.variableName, ss, strlen(ss)+1, 1, TID_STRING);
         }
      else if ((s = strstr(str, "IpAddress:")) != NULL)
         {
            char *ss =  (s + 10);
            while (isspace(*ss))
               ss++;
            if (ss[strlen(ss)-1]=='\n')
               ss[strlen(ss)-1]=0;
            
            //printf("%s = IpAddress value [%s]\n", name, ss);
            db_set_value(hDB, hRdb, name, ss, strlen(ss)+1, 1, TID_STRING);
         }
      else if ((s = strstr(str, " = \"\"")) != NULL)
	 {
	    db_set_value(hDB, hRdb, name, "", 1, 1, TID_STRING);
	 }
      else
	 {
	    printf("%s = unknown data type: %s", name, str);
	 }
    }

  pclose(fp);

  if (gCommError==0)
     set_equipment_status(eq_name, "Ok", "#00FF00");
  else
     set_equipment_status(eq_name, "Communication problem", "#800000");

  if (0) {
     update_settings();
     return 0;
  }

  return 0;
}

extern "C" INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
  assert(!"poll_event is not implemented");
  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

//end
