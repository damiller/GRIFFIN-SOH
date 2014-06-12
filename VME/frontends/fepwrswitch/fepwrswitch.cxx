/********************************************************************\

  Name:         fepwrswitch.cxx
  Created by:   K.Olchanski

  Contents:     Frontend for the MSCB DCC power switch box

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

#include "midas.h"
#include "mscb.h"
#include "evid.h"
#include "strlcpy.h"

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef FE_NAME
#define FE_NAME "fepwrswitch"
#endif

#ifndef EQ_NAME
#define EQ_NAME "PwrSwitch"
#endif

#ifndef EQ_EVID
#define EQ_EVID 1
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
   char *frontend_name = FE_NAME;
/* The frontend file name, don't change it */
   char *frontend_file_name = __FILE__;

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

/*-- Function declarations -----------------------------------------*/
  INT frontend_init();
  INT frontend_exit();
  INT begin_of_run(INT run_number, char *error);
  INT end_of_run(INT run_number, char *error);
  INT pause_run(INT run_number, char *error);
  INT resume_run(INT run_number, char *error);
  INT frontend_loop();
  
  INT read_pwrswitch_event(char *pevent, INT off);

/*-- Equipment list ------------------------------------------------*/
  
  EQUIPMENT equipment[] = {

    {EQ_NAME "%02d",          /* equipment name */
     {EQ_EVID, (1<<EQ_EVID),  /* event ID, trigger mask */
      "SYSTEM",               /* event buffer */
      EQ_PERIODIC|EQ_EB,      /* equipment type */
      0,                      /* event source */
      "MIDAS",                /* format */
      TRUE,                   /* enabled */
      RO_RUNNING|RO_STOPPED|RO_PAUSED|RO_ODB,       /* when to read this event */
      100,                    /* poll time in milliseconds */
      0,                      /* stop run after this event limit */
      0,                      /* number of sub events */
      60,                     /* whether to log history */
      "", "", "",}
     ,
     read_pwrswitch_event,         /* readout routine */
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

static std::string gMscbDev;

/*-- Frontend Init -------------------------------------------------*/

extern "C" int frontend_index;

static char eq_name[256];

INT frontend_init()
{
  setbuf(stdout,NULL);
  setbuf(stderr,NULL);

  if (frontend_index < 1)
    {
      cm_msg(MERROR, frontend_name, "frontend_init(): Frontend index %d is not valid, please start with \'-i 1\'", frontend_index);
      return !SUCCESS;
    }

  cm_set_transition_sequence(TR_START, 0);
  cm_set_transition_sequence(TR_STOP, 0);
  cm_set_transition_sequence(TR_PAUSE, 0);
  cm_set_transition_sequence(TR_RESUME, 0);

  sprintf(eq_name, "%s%02d", EQ_NAME, frontend_index);

  return SUCCESS;
}

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

#undef WORD_SWAP
#undef DWORD_SWAP

#include "msystem.h"

static void createBank(const char* eq_name, const char* bank_name, const char* init_str[])
{
  char str[256];
  sprintf(str, "/Equipment/%s/Variables/%s", eq_name, bank_name);
  int status = db_check_record(hDB, 0, str, strcomb(init_str), TRUE);
  if (status == DB_OPEN_RECORD) {
    cm_msg(MERROR, frontend_name, "Cannot check/create record \"%s\" because mlogger is holding it, Stop mlogger and try again.\n", str);
    exit(1);
  }

  if (status != DB_SUCCESS) {
    printf("Cannot check/create record \"%s\", status = %d\n", str, status);
    assert(!"Failed!");
  }
}

extern HNDLE hDB;

static int gIsInitialized = 0;
static int gMscbFd = 0;
static int gDebug = 0;
static int gMscbAddr = 0;
static char gAlarmName[256];
static char gBankName[256];

#include "MscbDevice.h"
#include "mscb_PWRSWITCH_2490.h"

static MscbDevice* gDriver;

static int gControlEnabled = 0;
static int gReadPeriod = 0;
static time_t gNextRead = 0;
static int gFastRead = 0;

static int gDemandControl = 0;

static void update_settings(INT a, INT b, void *c)
{
  printf("update_settings!\n");

  char str[1024];

  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/ReadPeriod", frontend_index);
  gReadPeriod = odbReadInt(str, 0, 10);

  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/control_enable", frontend_index);
  gControlEnabled = odbReadBool(str, 0, 0);

  if (gControlEnabled)
    printf("power switch control is enabled\n");
  else
    printf("power switch control is disabled - running in monitoring and history mode\n");

  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/TempLimit", frontend_index);
  odbReadFloat(str, 5, 0);

  for (int i=0; i<6; i++)
    {
      double limit = odbReadFloat(str, i, 0);
      if (limit>0)
	{
	  printf("write temperature %d limit %f degC\n", i+1, limit);

	  if (gControlEnabled)
	    gDriver->WriteFloat(gMscbFd, PWRSWITCH_2490_LTEMP1 + i, limit);
	}
    }

#if 0  
  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/Control", frontend_index);
  int control = odbReadInt(str);

  printf("write control 0x%x\n", control);

  if (gControlEnabled)
    gDriver->WriteByte(gMscbFd, PWRSWITCH_2490_CONTROL, control);
#endif

  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/Power", frontend_index);
  odbReadBool(str, 5);

  int control = 0;
  for (int i=0; i<6; i++)
    {
      if (odbReadBool(str, i))
	control |= (1<<i);
    }

  printf("write control 0x%x\n", control);

  if (gControlEnabled)
    {
      static int gPrevControl = -1;
      if (control != gPrevControl)
	cm_msg(MINFO, frontend_name, "Writing DCC power control register value 0x%02x", control);
      gPrevControl = control;

      gDriver->WriteByte(gMscbFd, PWRSWITCH_2490_CONTROL, control);

      gDemandControl = control;
    }

  gNextRead = time(NULL);
  gFastRead = 10;
}

// MIDAS ODB helpers

static HNDLE odbGetHandle(HNDLE hDB, HNDLE dir, const char *name, int type)
{
  /* Find key in the ODB */
  HNDLE key;
  int status = db_find_key (hDB, dir, name, &key);
  if (type != 0 && status == DB_NO_KEY)
    {
      // create key if it does not exist
      status = db_create_key(hDB, dir, name, type);
      if (status != SUCCESS)
	{
	  cm_msg (MERROR, frontend_name, "Cannot create \'%s\', error %d", name, status);
	  abort();
	  // NOT REACHED
	  return 0;
	}
      status = db_find_key(hDB, dir, name, &key);
      cm_msg (MERROR, frontend_name, "Created ODB key \'%s\' of type %d", name, type);
    }

  if (status != SUCCESS)
    {
      if (dir==0)
	cm_msg (MERROR, frontend_name, "Cannot find odb handle for \'%s\', error %d", name, status);
      else
	cm_msg (MERROR, frontend_name, "Cannot find odb handle for %d/\'%s\', error %d", dir, name, status);
      abort();
      // NOT REACHED
      return 0;
    }

  return key;
}

static void open_hotlink()
{
  static bool once = false;
  if (once)
    return;
  once = true;

  char str[1024];
  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings", frontend_index);

  int status = db_open_record(hDB, odbGetHandle(hDB, 0, str, 0), NULL, 0, MODE_READ, update_settings, NULL);
  assert(status == DB_SUCCESS);
}

int init_mscb()
{
  //cm_set_watchdog_params (FALSE, 0);

  set_equipment_status(eq_name, "Initializing...", "yellow");

  char str[1024];
  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/Device", frontend_index);

  gMscbDev = odbReadString(str, 0, "", 200);

  if (gMscbDev.length() < 2)
    {
      cm_msg(MERROR, frontend_name, "frontend_init(): MSCB device name is blank, please set \'%s\'", str);
      return !SUCCESS;
    }

  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/MSCB_Addr", frontend_index);
  gMscbAddr = odbReadInt(str);

  if (gMscbAddr < 1)
    {
      cm_msg(MERROR, frontend_name, "frontend_init(): MSCB address is blank, please set \'%s\'", str);
      return !SUCCESS;
    }

  sprintf(str, "/Equipment/" EQ_NAME "%02d/Settings/Debug", frontend_index);
  gDebug = odbReadInt(str);

  /* Open the Device */
  const char* devname = gMscbDev.c_str();

  if (strlen(devname) < 2) {
    cm_msg(MERROR, "init_mscb", "MSCB device name is NULL");
    return FE_ERR_HW;
  }

  /* open device on MSCB */

  printf("Connecting to MSCB bus \'%s\'\n", devname);

  if (gMscbFd > 0) {
    mscb_exit(gMscbFd);
    gMscbFd = 0;
  }

  int fd = mscb_init((char*)devname, NAME_LENGTH, "", FALSE);
  if (fd < 0) {
    cm_msg(MERROR, "init_mscb",
	   "Cannot initialize MSCB interface \"%s\", mscb_init() error %d",
	   devname,
           fd);
    return FE_ERR_HW;
  }

  int status = mscb_subm_info(fd);
  if (status != MSCB_SUCCESS) {
    cm_msg(MERROR, "init_mscb",
	   "Cannot initialize MSCB interface \"%s\", mscb_subm_info() error %d",
	   devname,
           status);
    return FE_ERR_HW;
  }

  gMscbFd = fd;

  cm_msg(MINFO, "init_mscb", "Connected to MSCB interface \'%s\'", devname);

  gDriver = new MscbDevice("PWRSWITCH", gMscbAddr);

  status = gDriver->Init(gMscbFd, PWRSWITCH_2490_NODE_NAME, PWRSWITCH_2490_NUM_VARS, 1700, 0, PWRSWITCH_2490_BUFSIZE);
  if (status == SUCCESS) {
    //fIsPresent = true;
  } else if (status == FE_ERR_HW) {
    cm_msg(MERROR, "PwrSwitchFe::Init", "PwrSwitch at MSCB address %d is missing", gDriver->fAddr);
    return !SUCCESS;
  } else {
    cm_msg(MERROR, "PwrSwitchFe::Init", "PwrSwitch at MSCB address %d is incompatible", gDriver->fAddr);
    return !SUCCESS;
  }

  sprintf(gBankName, "GP%02d", frontend_index);

  createBank(eq_name, gBankName, PWRSWITCH_2490_InitStr);

  update_settings(0,0,0);

  open_hotlink();

  set_equipment_status(eq_name, "Init Ok", "#00FF00");

  return SUCCESS;
}

int try_init_mscb(time_t now)
{
  static time_t gNextTry = 0;
  static int gRetry = 1;

  if (gNextTry == 0)
    gNextTry = now - 10;
  
  if (now < gNextTry)
    return -1;

  sprintf(gAlarmName, "%s%02d", frontend_name, frontend_index);
  
  int status = init_mscb();
  if (status != SUCCESS)
    {
      gNextTry = now + gRetry;
      
      if (gRetry <= 1*60*60)
        gRetry = 2*gRetry;
      
      char str[256];
      sprintf(str, "Init failed, retry in %d sec", gRetry);
      set_equipment_status(eq_name, str, "#FF8080");

      if (gRetry > 15)
        {
	  char text[256];
	  sprintf(text,"%s%02d cannot initialize %s, will try again, see messages", frontend_name, frontend_index, gMscbDev.c_str());
          al_trigger_alarm(gAlarmName, text, "Warning", "", AT_INTERNAL);
        }
      
      return -1;
    }
  
  al_reset_alarm(gAlarmName);
  
  return SUCCESS;
}

int read_pwrswitch_event(char *pevent, INT off)
{
  time_t now;
  time(&now);

  if (now < gNextRead)
    return 0;

  if (gFastRead)
    {
      gFastRead--;
      gNextRead = now + 1;
    }
  else
    gNextRead += gReadPeriod;

  if (gNextRead < now)
    gNextRead = now + gReadPeriod;

  if (!gIsInitialized)
    {
      int status = try_init_mscb(now);
      if (status != SUCCESS)
	return 0;

      gIsInitialized = true;
    }

  printf("read!\n");

  PWRSWITCH_2490_bank *bankp;

  bk_init32(pevent);
  bk_create(pevent, gBankName, TID_STRUCT, &bankp);

  int status = gDriver->ReadBank(gMscbFd, bankp, sizeof(*bankp), PWRSWITCH_2490_copy);

  if (gDebug)
    printf("******* Read PwrSwitch status %d\n", status);

  if (status != SUCCESS) {
    cm_msg(MERROR, frontend_name, "Cannot read PwrSwitch on MSCB chain %s, address %d, mscb read() status %d", gMscbDev.c_str(), gMscbAddr, status);
    gIsInitialized = false;
    return 0;
  }

  bk_close(pevent, bankp+1);

  char str[256];
  int heartbeat = now % 1000;
  double maxtemp = 0;
  maxtemp = MAX(maxtemp, bankp->temp1);
  maxtemp = MAX(maxtemp, bankp->temp2);
  maxtemp = MAX(maxtemp, bankp->temp3);
  maxtemp = MAX(maxtemp, bankp->temp4);
  maxtemp = MAX(maxtemp, bankp->temp5);
  maxtemp = MAX(maxtemp, bankp->temp6);
  maxtemp = MAX(maxtemp, bankp->intemp1);
  maxtemp = MAX(maxtemp, bankp->intemp2);
  maxtemp = MAX(maxtemp, bankp->intemp3);

  static bool gAlarmActive = false;

  if (!gControlEnabled || (gDemandControl == bankp->status)) {
    //sprintf(str, "Ok, D/S/C/S/T/HB: 0x%02x/0x%02x/0x%02x/0x%02x/%.1f/%d", gDemandControl, bankp->shutdown, bankp->control, bankp->status, maxtemp, heartbeat);
    sprintf(str, "Ok 0x%02x, E 0x%02x, Temp %.1fC, HB %d", bankp->status, bankp->shutdown, maxtemp, heartbeat);
    if (gDemandControl != 0)
      set_equipment_status(eq_name, str, "#00FF00"); // green colour
    else
      set_equipment_status(eq_name, str, "#FFFFFF"); // white colour

    gAlarmActive = false;
  } else {
    sprintf(str, "Error, Demand: 0x%02x, Status: 0x%02x, Error: 0x%02x, MaxTemp: %.1fC, Heartbeat: %d", gDemandControl, bankp->status, bankp->shutdown, maxtemp, heartbeat);
    set_equipment_status(eq_name, str, "yellow");

    if (!gAlarmActive) {
      cm_msg(MERROR, frontend_name, "Mismatch between Demand: 0x%02x and Status: 0x%02x, Error: 0x%02x, MaxTemp: %.1fC", gDemandControl, bankp->status, bankp->shutdown, maxtemp);

      char text[256];
      sprintf(text,"Check %s status", eq_name);
      al_trigger_alarm(gAlarmName, text, "Warning", "", AT_INTERNAL);

      gAlarmActive = true;
    }
  }

  return bk_size(pevent);
}

extern "C" INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
  assert(!"poll_event is not implemented");
  return 0;
}

//end
