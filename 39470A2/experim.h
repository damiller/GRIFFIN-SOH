/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Wed Jan  7 14:52:32 2009

\********************************************************************/

#ifndef EXCL_AGILENT34970A

#define AGILENT34970A_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
} AGILENT34970A_COMMON;

#define AGILENT34970A_COMMON_STR(_name) char *_name[] = {\
"[.]",\
"Event ID = WORD : 8",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 33",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 511",\
"Period = INT : 120000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] syrinx",\
"Frontend name = STRING : [32] THERfe",\
"Frontend file name = STRING : [256] frontend.c",\
"",\
NULL }

#define AGILENT34970A_SETTINGS_DEFINED

typedef struct {
  struct {
    DWORD     counter;
    DWORD     interval;
    DWORD     max;
  } watchdog;
} AGILENT34970A_SETTINGS;

#define AGILENT34970A_SETTINGS_STR(_name) char *_name[] = {\
"[Watchdog]",\
"Counter = DWORD : 0",\
"Interval = DWORD : 5",\
"Max = DWORD : 0",\
"",\
NULL }

#endif

#ifndef EXCL_HVMONITOR

#define HVMONITOR_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
} HVMONITOR_COMMON;

#define HVMONITOR_COMMON_STR(_name) char *_name[] = {\
"[.]",\
"Event ID = WORD : 8",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 33",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 511",\
"Period = INT : 10000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] tigsoh01.triumf.ca",\
"Frontend name = STRING : [32] HVMon",\
"Frontend file name = STRING : [256] frontend.c",\
"",\
NULL }

#define HVMONITOR_SETTINGS_DEFINED

typedef struct {
  struct {
    struct {
      INT       channels[12];
    } slot0;
    struct {
      INT       channels[12];
    } slot1;
  } tighv00;
  struct {
    struct {
      INT       channels[12];
    } slot0;
    struct {
      INT       channels[12];
    } slot2;
  } tighv01;
  struct {
    struct {
      INT       channels[12];
    } slot0;
    struct {
      INT       channels[12];
    } slot1;
  } tighv02;
} HVMONITOR_SETTINGS;

#define HVMONITOR_SETTINGS_STR(_name) char *_name[] = {\
"[tighv00/Slot0]",\
"Channels = INT[12] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"",\
"[tighv00/Slot1]",\
"Channels = INT[12] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"",\
"[tighv01/Slot0]",\
"Channels = INT[12] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"",\
"[tighv01/Slot2]",\
"Channels = INT[12] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"",\
"[tighv02/Slot0]",\
"Channels = INT[12] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"",\
"[tighv02/Slot1]",\
"Channels = INT[12] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"",\
NULL }

#endif

#ifndef EXCL_EPICS

#define EPICS_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
} EPICS_COMMON;

#define EPICS_COMMON_STR(_name) char *_name[] = {\
"[.]",\
"Event ID = WORD : 5",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 16",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 511",\
"Period = INT : 1000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] tigsoh01.triumf.ca",\
"Frontend name = STRING : [32] Epics",\
"Frontend file name = STRING : [256] frontend.c",\
"",\
NULL }

#define EPICS_SETTINGS_DEFINED

typedef struct {
  struct {
    INT       epics;
  } channels;
  struct {
    struct {
      char      channel_name[20][32];
    } epics;
  } devices;
  char      names[20][32];
  float     update_threshold_measured[20];
  char      keys[20][80];
} EPICS_SETTINGS;

#define EPICS_SETTINGS_STR(_name) char *_name[] = {\
"[Channels]",\
"Epics = INT : 20",\
"",\
"[Devices/Epics]",\
"Channel name = STRING[20] :",\
"[32] TIG1:SOH:SHACKTEMP",\
"[32] TIG1:SOH:VAR2",\
"[32] TIG1:SOH:VAR3",\
"[32] TIG1:SOH:VAR4",\
"[32] TIG1:SOH:VAR5",\
"[32] TIG1:SOH:VAR6",\
"[32] TIG1:SOH:VAR7",\
"[32] TIG1:SOH:VAR8",\
"[32] TIG1:SOH:VAR9",\
"[32] TIG1:SOH:VAR10",\
"[32] TIG1:SOH:VAR11",\
"[32] TIG1:SOH:VAR12",\
"[32] TIG1:SOH:VAR13",\
"[32] TIG1:SOH:VAR14",\
"[32] TIG1:SOH:VAR15",\
"[32] TIG1:SOH:VAR16",\
"[32] TIG1:SOH:VAR17",\
"[32] TIG1:SOH:VAR18",\
"[32] TIG1:SOH:VAR19",\
"[32] TIG1:SOH:WDIN",\
"",\
"[.]",\
"Names = STRING[20] :",\
"[32] Temperature of Shack 1",\
"[32] Temperature of Shack 2",\
"[32] Temp Readout",\
"[32] Hum Readout",\
"[32] Default%CH 4",\
"[32] Default%CH 5",\
"[32] Default%CH 6",\
"[32] Default%CH 7",\
"[32] Default%CH 8",\
"[32] Default%CH 9",\
"[32] Si V1",\
"[32] Si V2",\
"[32] Si I1",\
"[32] Si I2",\
"[32] Default%CH 14",\
"[32] Default%CH 15",\
"[32] Default%CH 16",\
"[32] Default%CH 17",\
"[32] Default%CH 18",\
"[32] Watchdog",\
"Update Threshold Measured = FLOAT[20] :",\
"[0] 0.1",\
"[1] 0.1",\
"[2] 0.01",\
"[3] 0.01",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"[16] 1",\
"[17] 1",\
"[18] 1",\
"[19] 0.5",\
"keys = STRING[20] :",\
"[80] /Equipment/Agilent34970A/Variables/DATA[0]",\
"[80] /Equipment/Agilent34970A/Variables/DATA[1]",\
"[80] /Equipment/Agilent34970A/Variables/DATA[2]",\
"[80] /Equipment/Agilent34970A/Variables/DATA[3]",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] /Equipment/Mesytec MHV-4/Variables/DATA[0]",\
"[80] /Equipment/Mesytec MHV-4/Variables/DATA[2]",\
"[80] /Equipment/Mesytec MHV-4/Variables/DATA[4]",\
"[80] /Equipment/Mesytec MHV-4/Variables/DATA[6]",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] ",\
"[80] /Equipment/Agilent34970A/Settings/Watchdog/Counter",\
"",\
NULL }

#endif

#ifndef EXCL_MESYTEC_MHV_4

#define MESYTEC_MHV_4_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
} MESYTEC_MHV_4_COMMON;

#define MESYTEC_MHV_4_COMMON_STR(_name) char *_name[] = {\
"[.]",\
"Event ID = WORD : 9",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 511",\
"Period = INT : 10000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] tigsoh01.triumf.ca",\
"Frontend name = STRING : [32] DetectorBiasMon",\
"Frontend file name = STRING : [256] frontend.c",\
"",\
NULL }

#endif

