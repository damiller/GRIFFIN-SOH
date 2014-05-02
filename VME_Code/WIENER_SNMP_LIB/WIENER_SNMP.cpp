/** WIENER SNMP basic SNMP library to Demonstrate C-Access to WIENER-Crates via SNMP
// modified for LabView imprt 04/23/06, Andreas Ruben
//
// The path to the Net-SNMP include files (default /usr/include) must be added to the 
// include file search path!
// The following libraries must be included:
// netsnmp.lib ws2_32.lib
// The path to the Net-SNMP library must be added to the linker files.
// /usr/lib
// path for the WIENER MIB file (mibdirs) c:/usr/share/snmp/mibs
*/



#include "WIENER_SNMP.h"
//#include <windows.h>




/******************************************************************************/
/** simple syslog replacement for this example.
 ** commented out by Greg and it seems to make things work.
 */
#if 0
void syslog(int priority,const char* format,...) {
  va_list vaPrintf; 
  va_start(vaPrintf,format);
  vprintf(format,vaPrintf); putchar('\n');
  va_end(vaPrintf);
}
#endif

/******************************************************************************/
/** SNMP Initialization.
*/


int SnmpInit(void) {
	syslog(LOG_DEBUG,"*** Initialise SNMP ***");

  init_snmp("CrateTest");									  // I never saw this name used !?!
	init_mib();																// init MIB processing
  if(!read_module("WIENER-CRATE-MIB")) {		// read specific mibs
    syslog(LOG_ERR,"Unable to load SNMP MIB file \"%s\"","WIENER-CRATE-MIB");
    return 0;
  }
  syslog(LOG_DEBUG,"*** Translate OIDs ***");
	
  lengthSysMainSwitch = MAX_OID_LEN;
  if(!get_node(S_sysMainSwitch,oidSysMainSwitch,&lengthSysMainSwitch)) {
		syslog(LOG_ERR,"OID \"sysMainSwitch.0\"not found"); return false;
  } 

  lengthOutputMeasurementSenseVoltageAll = MAX_OID_LEN;
  if(!get_node(S_outputMeasurementSenseVoltageAll,oidOutputMeasurementSenseVoltageAll,&lengthOutputMeasurementSenseVoltageAll)) {
		syslog(LOG_ERR,"OID \"sysMainSwitch.0\"not found"); return false;
  } 

  lengthFanAirTemperature = MAX_OID_LEN;
  if(!get_node(S_fanAirTemperature,oidFanAirTemperature,&lengthFanAirTemperature)) {
		syslog(LOG_ERR,"OID \"fanAirTemperature.0\"not found"); return false;
  } 

  for(int j = 0; j < 8; j++)
  {
	lengthSensorTemperature[j] = MAX_OID_LEN;
	if(!get_node(S_sensorTemperature[j],oidSensorTemperature[j],&lengthSensorTemperature[j])) {
		syslog(LOG_ERR,"OID \"sensorTemperature.0\"not found"); return false;
  } 
  }

  for(int j = 0; j < 12; j++)
  {
	lengthOutputMeasurementSenseVoltage[j] = MAX_OID_LEN;
	if(!get_node(S_outputMeasurementSenseVoltage[j],oidOutputMeasurementSenseVoltage[j],&lengthOutputMeasurementSenseVoltage[j])) {
		syslog(LOG_ERR,"OID \"outputMeasurementSenseVoltage.1\"not found"); return false;
	 }
	lengthOutputVoltage[j] = MAX_OID_LEN;
	if(!get_node(S_outputVoltage[j],oidOutputVoltage[j],&lengthOutputVoltage[j])) {
		syslog(LOG_ERR,"OID \"outputVoltage.1\"not found"); return false;
	}
	lengthOutputMeasurementCurrent[j] = MAX_OID_LEN;
		if(!get_node(S_outputMeasurementCurrent[j],oidOutputMeasurementCurrent[j],&lengthOutputMeasurementCurrent[j])) {
			syslog(LOG_ERR,"OID \"outputMeasurementCurrent.1\"not found"); return false;
		}
	lengthOutputStatus[j] = MAX_OID_LEN;
		if(!get_node(S_outputStatus[j],oidOutputStatus[j],&lengthOutputStatus[j])) {
			syslog(LOG_ERR,"OID \"outputStatus.1\"not found"); return false;
		} 
	lengthChannelSwitch[j] = MAX_OID_LEN;
		if(!get_node(S_outputSwitch[j],oidChannelSwitch[j],&lengthChannelSwitch[j])) {
			syslog(LOG_ERR,"OID \"channelSwitch.1\"not found"); return false;
		} 
//	lengthOutputRampUp[j] = MAX_OID_LEN;
//		if(!get_node(S_outputRampUp[j],oidOutputRampUp[j],&lengthOutputRampUp[j])) {
//			syslog(LOG_ERR,"OID \"outputRampUp.1\"not found"); return false;
//		} 
//	lengthOutputRampDown[j] = MAX_OID_LEN;
//		if(!get_node(S_outputRampDown[j],oidOutputRampDown[j],&lengthOutputRampDown[j])) {
//			syslog(LOG_ERR,"OID \"outputRampDown.1\"not found"); return false;
//		} 
  }


  syslog(LOG_DEBUG,"*** Initialise SNMP done ***");
  SOCK_STARTUP;															// only in main thread
  return 1;
}

/******************************************************************************/
/** SNMP Cleanup.
*/
void SnmpCleanup(void) {
	SOCK_CLEANUP;
}


/******************************************************************************/
/** SNMP Open Session.
*/
//typedef void* HSNMP;                              // SNMP handle (like FILE)

HSNMP SnmpOpen(const char* ipAddress) {
	HSNMP m_sessp;
  struct snmp_session session;
  snmp_sess_init(&session);			                  // structure defaults
  session.version = SNMP_VERSION_2c;
  session.peername = strdup(ipAddress);
  session.community = (u_char*)strdup(readCommunity);
  session.community_len = strlen((char*)session.community);

  session.timeout = 100000;   // timeout (us)
  session.retries = 3;        // retries

  if(!(m_sessp = snmp_sess_open(&session))) {
    int liberr, syserr;
    char *errstr;
    snmp_error(&session, &liberr, &syserr, &errstr);
    syslog(LOG_ERR,"Open SNMP session for host \"%s\": %s",ipAddress,errstr);
    free(errstr);
    return 0;
  }

//  m_session = snmp_sess_session(m_sessp);     // get the session pointer 

  syslog(LOG_INFO,"SNMP session for host \"%s\" opened",ipAddress);
  return m_sessp;
}

/******************************************************************************/
/** SNMP Close Session.
*/
void SnmpClose(HSNMP m_sessp) {
  if(!m_sessp) return;
  if(!snmp_sess_close(m_sessp)) {
    syslog(LOG_ERR,"Close SNMP session: ERROR");
  }
  else syslog(LOG_INFO,"SNMP session closed");
}

/******************************************************************************/
/** Get voltage from power supply
*/
double getOutputSenseMeasurement(HSNMP m_sessp, int channel) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputMeasurementSenseVoltage[channel],lengthOutputMeasurementSenseVoltage[channel]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}


/******************************************************************************/
/** Get voltage from power supply
*/
double getOutputVoltage(HSNMP m_sessp, int channel) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputVoltage[channel],lengthOutputVoltage[channel]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}


/******************************************************************************/
/** Write Voltage to power supply
*/
double  setOutputVoltage(HSNMP m_sessp,int channel,double value) {
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_SET);    // prepare set-request pdu
  pdu->community = (u_char*)strdup(writeCommunity);
  pdu->community_len = strlen(writeCommunity);

  // for(each SET request to one crate) {
  float v = (float) value;
  snmp_pdu_add_variable(pdu,oidOutputVoltage[channel],lengthOutputVoltage[channel],ASN_OPAQUE_FLOAT,(u_char*)&v,sizeof(v));
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}


/******************************************************************************/
/** Get current from power supply
*/
double getCurrentMeasurement(HSNMP m_sessp, int channel) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputMeasurementCurrent[channel],lengthOutputMeasurementCurrent[channel]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);


  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}





/******************************************************************************/
/** Get RampUp from power supply
*/
double getOutputRampUp(HSNMP m_sessp, int channel) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputRampUp[channel],lengthOutputRampUp[channel]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}



/******************************************************************************/
/** Write RampUp to power supply
*/
double  setOutputRampUp(HSNMP m_sessp,int channel,double value) {
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_SET);    // prepare set-request pdu
  pdu->community = (u_char*)strdup(writeCommunity);
  pdu->community_len = strlen(writeCommunity);

  // for(each SET request to one crate) {
  float v = (float) value;
  snmp_pdu_add_variable(pdu,oidOutputRampUp[channel],lengthOutputRampUp[channel],ASN_OPAQUE_FLOAT,(u_char*)&v,sizeof(v));
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}


/******************************************************************************/
/** Get RampDown from power supply
*/
double getOutputRampDown(HSNMP m_sessp, int channel) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputRampDown[channel],lengthOutputRampDown[channel]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}



/******************************************************************************/
/** Write RampDown to power powersupply
*/
double  setOutputRampDown(HSNMP m_sessp,int channel,double value) {
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_SET);    // prepare set-request pdu
  pdu->community = (u_char*)strdup(writeCommunity);
  pdu->community_len = strlen(writeCommunity);

  // for(each SET request to one crate) {
  float v = (float) value;
  snmp_pdu_add_variable(pdu,oidOutputRampDown[channel],lengthOutputRampDown[channel],ASN_OPAQUE_FLOAT,(u_char*)&v,sizeof(v));
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}




/******************************************************************************/
/** Get on/off status of crate
*/
int getMainSwitch(HSNMP m_sessp) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidSysMainSwitch,lengthSysMainSwitch);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return (int) value;
}



/******************************************************************************/
/** Write on/off status
*/
double  setMainSwitch(HSNMP m_sessp,float value) {
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_SET);    // prepare set-request pdu
  pdu->community = (u_char*)strdup(writeCommunity);
  pdu->community_len = strlen(writeCommunity);

  // for(each SET request to one crate) {
  int v = (int) value;
  snmp_pdu_add_variable(pdu,oidSysMainSwitch,lengthSysMainSwitch,ASN_INTEGER,(u_char*)&v,sizeof(v));
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}


/******************************************************************************/
/** Get channel status 
*/
int getChannelStatus(HSNMP m_sessp, int channel) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputStatus[channel],lengthOutputStatus[channel]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }		else if(vars->type == ASN_OCTET_STR) {				        // 0x04
				unsigned long bitstring = 0;
				for(int cpos = vars->val_len-1;cpos >= 0;cpos--) {
					unsigned char octet = vars->val.string[cpos];
					for(int bpos = 0;bpos < 8;bpos++) {		// convert one character
					bitstring <<= 1;
						if(octet&0x01) bitstring |= 1;
						octet >>= 1;
					}
				}
        value = bitstring;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return (int) value;
}

double setChannelSwitch(HSNMP m_sessp, int channel,float value) {
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_SET);    // prepare set-request pdu
  pdu->community = (u_char*)strdup(writeCommunity);
  pdu->community_len = strlen(writeCommunity);

  // for(each SET request to one crate) {
  int v = (int) value;
  snmp_pdu_add_variable(pdu,oidChannelSwitch[channel],lengthChannelSwitch[channel],ASN_INTEGER,(u_char*)&v,sizeof(v));
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return value;
}


/******************************************************************************/
/** Get Fantray Temp
*/
int getFantrayTemp(HSNMP m_sessp) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidFanAirTemperature,lengthFanAirTemperature);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return (int) value;
}


/******************************************************************************/
/** Get TempSensor
*/
int getSensorTemp(HSNMP m_sessp, int sensor) {
  double value;

  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);    // prepare get-request pdu

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidSensorTemperature[sensor],lengthSensorTemperature[sensor]);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
    for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        value = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        value = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				value = (double)*vars->val.integer;
      }
    }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return (int) value;
}




/******************************************************************************/
/** Get voltage from power supply
*/
double getAllOutputSenseMeasurement(HSNMP m_sessp, double values[]) {


  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GETBULK);    // prepare get-request pdu
  pdu->max_repetitions = 12;
  pdu->non_repeaters = 12;

  // for(each GET request to one crate) {
    snmp_add_null_var(pdu,oidOutputMeasurementSenseVoltageAll,lengthOutputMeasurementSenseVoltageAll);   // generate request data
  // } // endfor

  struct snmp_pdu* response;
	int status = snmp_sess_synch_response(m_sessp,pdu,&response);

  /*
  * Process the response.
  */
  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    /*
    * SUCCESS: Print the result variables
    */
    struct variable_list *vars;
    
    // debug print
    //for(vars = response->variables; vars; vars = vars->next_variable)
    //  print_variable(vars->name, vars->name_length, vars);

    /* manipuate the information ourselves */
  
  
  int i=0; 
	for (i=0; i<12; i++);
	{
	for(vars = response->variables; vars; vars = vars->next_variable) {
			if (vars->type == ASN_OPAQUE_FLOAT) {				    // 0x78
        values[i] = *vars->val.floatVal;
      }
			else if (vars->type == ASN_OPAQUE_DOUBLE) {			// 0x79
        values[i] = *vars->val.doubleVal;
      }
			else if(vars->type == ASN_INTEGER) {				      // 0x02
				values[i] = (double)*vars->val.integer;
      }
	}
  }
  } else {
    /*
    * FAILURE: print what went wrong!
    */

    if (status == STAT_SUCCESS)
      fprintf(stderr, "Error in packet\nReason: %s\n",
      snmp_errstring(response->errstat));
    else
      snmp_sess_perror("snmpget",snmp_sess_session(m_sessp));
    return 0;
  }
  snmp_free_pdu(response);


  return 1;
}
