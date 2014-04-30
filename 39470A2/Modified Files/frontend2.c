/********************************************************************\

  Name:         frontend.c
  Created by:   Renee Poutissou

  Modified by pewg for new setup and new MIDAS 1.8.2 (10 Feb 2001)
  Contents:     Simple frontend code with 1 event 
\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "midas.h"
#include "mcstd.h"
/*#include "experim.h"*/

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "THERfe";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
  INT display_period = 0;
/*INT display_period = 3000;*/

/* maximum event size produced by this frontend */
INT max_event_size = 10000;
  /* added per instructions on ladd00*/
  INT max_event_size_frag = 5*1024*1024;

/* buffer size to hold events */
INT event_buffer_size = 10*1024*1024;

/* number of channels */
#define N_ther 23
#define Add_start 101

  char resp[512];
  int  hDev = 0, nbytes=0, status;
  

/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_ther_event(char *pevent, INT off);

/*-- Equipment list ------------------------------------------------*/

#undef USE_INT

EQUIPMENT equipment[] = {

  { "Agilent 34970A",         /* equipment name */
    8, 0,                 /* event ID, trigger mask */
    "SYSTEM",             /* event buffer */
    EQ_PERIODIC,          /* equipment type */
    0,                    /* event source */
    "MIDAS",              /* format */
    TRUE,                 /* enabled */
/*    RO_RUNNING |
/*    RO_TRANSITIONS |      /* read when running and on transitions */
/*    RO_ODB,              /* and update ODB */ 
    511,                  /* Read out under all circumstances */	
    60000,                /* read every 60 sec */
    0,                    /* stop run after this event limit */
    0,                    /* number of sub events */
    1,                    /* log history */
    "", "", "",
    read_ther_event,    /* readout routine */
  },

  { "" },

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

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
  /* hardware initialization */

   hDev = rs232_init(0, 9600, 'E', 7, 1, 0);
 
  if (hDev)
  {
    printf("Init starting\n");
/*    rs232_puts(hDev, "ROUT:MON,(@103)\r\n");*/
    
      rs232_puts(hDev, "CONF:RES AUTO,(@105)\r\n");
      rs232_puts(hDev, "CONF:TEMP RTD,85,(@101)\r\n");	
    
 /*   rs232_puts(hDev, "*RST\r\n");
    rs232_puts(hDev, "*CLS\r\n");
    rs232_puts(hDev, "CONF:TEMP THER,10000,(@101)\r\n");*/
    
    printf ("Init Done\n");
    return SUCCESS;
  }
  else
  {
    cm_msg(MERROR, "frontend_init"," cannot initialize RS232 to DVM");
    return FE_ERR_HW;
  }
  /* print message and return FE_ERR_HW if frontend should not be started */

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
  /* put here clear scalers etc. */

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
  /* if frontend_call_loop is true, this routine gets called when
     the frontend is idle or once between every event */
  return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\
  
  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
int   i;
DWORD dummy;

  dummy = 0; 
  for (i=0 ; i<count ; i++)
    {
    if (dummy == 1)
      if (!test)
        return 1;
    }

  return 0;
}

/*-- Interrupt configuration ---------------------------------------*/

INT interrupt_configure(INT cmd, INT source, PTYPE adr)
{
  switch(cmd)
    {
    case CMD_INTERRUPT_ENABLE:
      break;
    case CMD_INTERRUPT_DISABLE:
      break;
    case CMD_INTERRUPT_ATTACH:
      break;
    case CMD_INTERRUPT_DETACH:
      break;
    }
  return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/


INT read_ther_event(char *pevent, INT off)
{
  //return(0);
  float *pdata;
  INT a;
  char *endptr;
  char *sptr;
  double fdat=-1;
  
  rs232_puts(hDev, "MEAS:TEMP? RTD,(@101)\r\n");
  /*rs232_puts(hDev, "MEAS:RES? AUTO,(@105)\r\n");*/
  nbytes = rs232_gets_nowait(hDev, resp, 512, 0); /* was wait, now nowait */
  
  /*printf("\nreceiving: nbytes:%d\n|%s|\n",nbytes, resp);*/
  
  /* init bank structure */
  bk_init(pevent);
  
  /* create Temperature bank "TEMP" */
  bk_create(pevent, "TEMP", TID_FLOAT, &pdata);
  
  /* fill  bank */
  sptr = resp + 1;
  printf("Start loop on string%s\n\n",resp);
  for (a=0 ; a<N_ther ; a++)
  {
    /* fdat = strtod(sptr, &endptr);
    printf("data is %f\n",fdat);*/
    *pdata++ = (float) strtod(sptr, &endptr);
    sptr = endptr + 1;
  }
  bk_close(pevent, pdata);
  
  return bk_size(pevent);
}
