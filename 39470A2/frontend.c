/********************************************************************\

 Recent changes:
 
 GH: Added a front-end loop as an independent watchdoggie.
 
  Name:         frontend.c
  Created by:   Renee Poutissou

  Modified by pewg for new setup and new MIDAS 1.8.2 (10 Feb 2001)
  Contents:     Simple frontend code with 1 event 
\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "midas.h"
#include "mcstd.h"
/* #include <time.h> */
/*#include "experim.h"*/

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef VERBOSE
#define VERBOSE 0
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
   char *frontend_name = "THERfe";
/* The frontend file name, don't change it */
   char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
   BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms    */
   INT display_period = 000;
/* This variable is needed even if you don't use it. */

/* maximum event size produced by this frontend */
   INT max_event_size = 10000;
/* added per instructions on ladd00*/
   INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
   INT event_buffer_size = 10 * 1024 * 1024;

/* number of channels */
#define N_ther 22
#define Add_start 101

   char resp[512] = { "0.0" };
   int hDev = 0, nbytes = 0, status;
   const int channelList[] = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115};


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
     {"Agilent34970A", {        /* equipment name */
	 8, 0,                    /* event ID, trigger mask */
	 "SYSTEM",                /* event buffer */
	 EQ_PERIODIC | EQ_MANUAL_TRIG,    /* equipment type */
	 0,                       /* event source */
	 "MIDAS",                 /* format */
	 TRUE,                    /* enabled */
	 RO_ALWAYS,               /* Read out under all circumstances */
	 10000,                  /* read every 120 sec */
	 0,                       /* stop run after this event limit */
	 0,                       /* number of sub events */
	 1,                       /* log history */
	 "", "", "",},
      read_ther_event,         /* readout routine */
     },

     {""},
     
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

   hDev = rs232_init(0, 57600, 'N', 8, 1, 0);

   if (hDev) {
      printf("Init starting\n");
      //Clear panel
      rs232_puts(hDev, "*CLS\n");

      //Reset Channel Configurations
      //  rs232_puts(hDev, "*RST\r\n");

      //Scan List Initialization   

      rs232_puts(hDev, "CONF:TEMP RTD,85,(@101)\n");
      rs232_puts(hDev, "CONF:TEMP RTD,85,(@102)\n");
      rs232_puts(hDev, "CONF:VOLT:DC (@103:115)\n");
      // rs232_puts(hDev, "CALC:SCAL:GAIN 25,(@103)\r\n");
      //rs232_puts(hDev, "CALC:SCAL:OFFS 25,(@103)\r\n");
      //rs232_puts(hDev, "CALC:SCAL:GAIN 18.75,(@104)\r\n");
      // rs232_puts(hDev, "CALC:SCAL:OFFS 33.75,(@104)\r\n");
      //rs232_puts(hDev, "CALC:SCAL:STATE ON,(@103,104)\r\n");
      rs232_puts(hDev, "ROUT:SCAN (@101:115)\n");


/*   rs232_puts(hDev, "*RST\r\n");
    rs232_puts(hDev, "*CLS\r\n");
    rs232_puts(hDev, "CONF:TEMP THER,10000,(@101)\r\n");*/

      printf("Init Done\n");
      return SUCCESS;
   } else {
      cm_msg(MERROR, "frontend_init", " cannot initialize RS232 to DVM");
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
/* This loop increments a "Watchdog" counter.               */
/* It does it every few seconds per an odb entry.           */
/* A separate subroutine can reset the watchdog countup,    */
/* or that can be done within this loop if the countup hits */
/* some limit.                                              */
/* This variable is deposited in an ODB entry after it is   */
/* updated so that can be alarm-monitored.                  */

static DWORD watchdog_counter = 0;      /* Last time the watchdog was updated */
static DWORD watchdog_interval = 1;
static DWORD watchdog_max = 0;  /* Set to 0 if you want readout routine to do it */
                                  /* or to 0xffffffff if you want it to go really high */
static HNDLE hDB = 0;

INT frontend_loop()
{
   static hInterval = 0, hMax = 0, hCounter = 0; /* handles for getting info */
   static INT max = 16, last_update = 0;
   DWORD temp;                  /* so you don't corrupt a perfectly good value */
   INT blah, now_time;          /* Temporary value */
   INT status;
   char set_str[MAX_ODB_PATH];
   char watchdog_str[MAX_ODB_PATH];

   /* Yield for a while to not use all CPU cycles */
   cm_yield(250);               /* similar to EPICS dwell time */
   if (!hDB) {
      /* Block of code for getting handles */
      if (cm_get_experiment_database(&hDB, NULL) != CM_SUCCESS) {
         cm_msg(MERROR, "frontend_loop",
                "Could not connect to experiment database!  WTF?");
         return FE_ERR_HW;
      }      
      sprintf(watchdog_str, "/Equipment/Agilent34970A/Settings/Watchdog");

      sprintf(set_str, "%s/Counter", watchdog_str);
      if (db_find_key(hDB, 0, set_str, &hCounter) != DB_SUCCESS) {
	 status = db_set_value(hDB, 0, set_str, &blah, sizeof(DWORD), 1, TID_DWORD);
	 if (status != DB_SUCCESS) {
	    cm_msg(MERROR, "frontend_loop", "Failed to create Watchdog counter.");
	    return FE_ERR_HW;
	 }
         cm_msg(MINFO, "frontend_loop", "Could not find Watchdog Counter -- it has been created");	 
	 db_find_key(hDB, 0, set_str, &hCounter);
      }
      
      sprintf(set_str, "%s/Interval", watchdog_str);
      if (db_find_key(hDB, 0, set_str, &hInterval) != DB_SUCCESS) {
	 blah = 5;
	 status = db_set_value(hDB, 0, set_str, &blah, sizeof(DWORD), 1, TID_DWORD);
	 if (status != DB_SUCCESS) {
	    cm_msg(MERROR, "frontend_loop", "Failed to create Watchdog interval.");
	    return FE_ERR_HW;
	 }
         cm_msg(MINFO, "frontend_loop", "Could not find Watchdog Interval -- it has been created");
	 db_find_key(hDB, 0, set_str, &hInterval);
      }

      sprintf(set_str, "%s/Max", watchdog_str);
      if (db_find_key(hDB, 0, set_str, &hMax) != DB_SUCCESS) {
	 blah = 0;
	 status = db_set_value(hDB, 0, set_str, &blah, sizeof(DWORD), 1, TID_DWORD);
	 if (status != DB_SUCCESS) {
	    cm_msg(MERROR, "frontend_loop", "Failed to create Watchdog max.");
	    return FE_ERR_HW;
	 }
         cm_msg(MINFO, "frontend_loop", "Could not find Watchdog Max -- it has been created");
	 db_find_key(hDB, 0, set_str, &hMax);
      }
   } // if the database handle has not been set

   /* At this point all the handles whould be defined, so start using 'em */
   /* Attempt to get interval.  Don't "epic fail" if you can't. */
   blah = sizeof(temp);
   if (db_get_data(hDB, hInterval, &temp, &blah, TID_DWORD) == DB_SUCCESS) {
      watchdog_interval = temp;
   }
   now_time = ss_time();        /* Get current time */
   if ((now_time - last_update) > watchdog_interval) {  /* If it's been a while since we updated the counter, */
      /* This block of code updates the watchdog timer */
      last_update = now_time;   /* Set the time appropriately... */
      /* Put the local counter to the ODB */
      db_set_data_index(hDB, hCounter, &watchdog_counter, sizeof(temp), 0,
                        TID_DWORD);
      /* Now put the local counter in a file, too */
      {
         FILE *fp;
         char fn[] = "/home1/tigsoh/THERfe.counter";
         if ((fp = fopen(fn, "w+")) == 0) {     /* open for creation & overwriting */
            cm_msg(MERROR, "frontend_loop", "Opening watchdog counter %s: %s", fn, strerror());
         } else {
            fprintf(fp, "%d\n", watchdog_counter);
            fclose(fp);
            if (chmod(fn, 0644)) {      /* returns 0 on good, -1 on error */
               cm_msg(MERROR, "frontend_loop", "Chmoding watchdog counter %s: %s", fn, strerror());
            }
         }
      }                         /* end of writing counter to a file */
      watchdog_counter++;       /* Increment the local counter */

      /* Now get max */
      if (db_get_data(hDB, hMax, &temp, &blah, TID_DWORD) == DB_SUCCESS) {
         watchdog_max = temp;
      }
      if (watchdog_max) {       /* ie. not equal to zero */
         if (watchdog_counter > watchdog_max) {
            watchdog_counter = 0;       /* reset to zero if it's too big */
         }
      }
   }
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
   int i;
   DWORD dummy;

   dummy = 0;
   for (i = 0; i < count; i++) {
      if (dummy == 1)
         if (!test)
            return 1;
   }

   return 0;
}

/*-- Interrupt configuration ---------------------------------------*/

INT interrupt_configure(INT cmd, INT source, PTYPE adr)
{
   switch (cmd) {
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
   float *pdata;
   int a, i;
   char *endptr;
   char *sptr;
   double fdat = -1;
   int match = 0;
   float tmp = 1000000;         /*initialized to some number greater than maximum spurious limit */
   float SPURIOUS[] = { 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0 };    /*first element corresponds to spurious limit for channel 101 */
   int nbr;                     /* number of bytes read on one attempt */

   if (!hDB) {
      // don't do anything until we have the database
      return 0;
   }
   /* Before you start: pat the dog and reset the watchdog, if necessary */
   if (!watchdog_max)
      watchdog_counter = 0;     /* see frontend_loop for these global variables */

   /*
   // Make phony data
   bk_init(pevent);
   bk_create(pevent, "DATA", TID_FLOAT, &pdata);
   for (i=0; i < N_ther; i++) {
      *pdata++ = (float)i;
   }
   bk_close(pevent, pdata);
   return bk_size(pevent);
   */

   rs232_puts(hDev, "READ?\n");
   //printf("Starting sleep\n");
   //fflush(stdout);
   /*sleep(4); /* Sleep for 4 seconds */
   //printf("Stopping sleep\n");
   //fflush(stdout); 
#if (0)
   //ss_sleep(10000);
   // usleep(10000000);
   // sleep(10);
   nbytes = rs232_gets_wait(hDev, resp, 512, '\n');      /* was wait, now nowait */
   printf("\nreceiving: nbytes:%d\n|%s|\n", nbytes, resp);
#else
   ss_sleep(4000);              /* ugh */
   nbytes = 0;
   i = 1;                      /* Max out at 1000 attempts ... */
   while ((i > 0)) {
      --i;
      ss_sleep(10);             /* ugh */
      nbr = rs232_gets_nowait(hDev, (resp + nbytes), 512, '\n');
      nbytes += nbr;
      if (VERBOSE) {
	 printf("Attempt %4d: nbr=%d, nbytes=%d, resp = |%s|\n",
		i, nbr, nbytes, resp);
      }
      if (nbytes == 0)
	 continue;
      if (nbr == 0)	
	 continue;
      if (resp[nbytes - 1] == '\n')
         break;
   }
#endif
   /* init bank structure */
   bk_init(pevent);

   /* create Temperature bank "TEMP" */
   bk_create(pevent, "DATA", TID_FLOAT, &pdata);

   /* fill  bank */
   sptr = resp;
   // printf("Start loop on string%s\n\n",resp);

   for (a = 0; a < N_ther; a++) {
      match = 0;
      //fdat = strtod(sptr, &endptr);
      //printf("data is %f\n",fdat);

      for (i = 0; i < (sizeof(channelList) / sizeof(int)); i++) {
         if ((101 + a) == channelList[i]) {
            match = 1;

	    tmp = (float) strtof(sptr, &endptr);
	    sptr = endptr + 1;
	    if (tmp < SPURIOUS[a]) {
	       *pdata = tmp;
	    } else {
	       *pdata = -99;
	    }
	    // printf("channel %d is %f\n", (a + 1), *pdata);
	    pdata++;
	    break;
         }
         tmp = 1000000;
      }
      if (match == 0) {
         *pdata = 0;
         pdata++;
      }
   }
   bk_close(pevent, pdata);

   return bk_size(pevent);
}
