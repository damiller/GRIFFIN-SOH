

#include <net-snmp/net-snmp-config.h>
#define NET_SNMP_SNMPV3_H                   // we don't need SNMPV3 (one include file is missing)
#include <net-snmp/net-snmp-includes.h>

typedef void* HSNMP;   // SNMP handle (like FILE)

const char* readCommunity = "public";       ///< community name for read operations
const char* writeCommunity = "guru";        ///< community name for write operation

const char *S_sysMainSwitch = "sysMainSwitch.0";
const char *S_fanAirTemperature ="fanAirTemperature.0";
const char *S_outputMeasurementSenseVoltageAll ="outputMeasurementSenseVoltage.0";

const char *S_sensorTemperature[8] = {"sensorTemperature.1",
								"sensorTemperature.2",
								"sensorTemperature.3",
								"sensorTemperature.4",
								"sensorTemperature.5",
								"sensorTemperature.6",
								"sensorTemperature.7",
								"sensorTemperature.8"};


const char *S_outputMeasurementSenseVoltage[12] ={"outputMeasurementSenseVoltage.1", 
										   "outputMeasurementSenseVoltage.2",
										   "outputMeasurementSenseVoltage.3",
										   "outputMeasurementSenseVoltage.4",
										   "outputMeasurementSenseVoltage.5",
										   "outputMeasurementSenseVoltage.6",
										   "outputMeasurementSenseVoltage.7",
										   "outputMeasurementSenseVoltage.8",
										   "outputMeasurementSenseVoltage.9",
										   "outputMeasurementSenseVoltage.10",
										   "outputMeasurementSenseVoltage.11",
										   "outputMeasurementSenseVoltage.12"};

const char *S_outputMeasurementCurrent[12] ={"outputMeasurementCurrent.1",
									  "outputMeasurementCurrent.2",
									  "outputMeasurementCurrent.3",
									  "outputMeasurementCurrent.4",
									  "outputMeasurementCurrent.5",
									  "outputMeasurementCurrent.6",
									  "outputMeasurementCurrent.7",
									  "outputMeasurementCurrent.8",
									  "outputMeasurementCurrent.9",
									  "outputMeasurementCurrent.10",
									  "outputMeasurementCurrent.11",
									  "outputMeasurementCurrent.12"};

const char *S_outputVoltage[12] ={"outputVoltage.1",
						   "outputVoltage.2",
						   "outputVoltage.3",
						   "outputVoltage.4",
						   "outputVoltage.5",
						   "outputVoltage.6",
						   "outputVoltage.7",
						   "outputVoltage.8",
						   "outputVoltage.9",
						   "outputVoltage.10",
						   "outputVoltage.11",
						   "outputVoltage.12"};

const char *S_outputRampUp[12] ={"outputRampUp.1",
						   "outputRampUp.2",
						   "outputRampUp.3",
						   "outputRampUp.4",
						   "outputRampUp.5",
						   "outputRampUp.6",
						   "outputRampUp.7",
						   "outputRampUp.8",
						   "outputRampUp.9",
						   "outputRampUp.10",
						   "outputRampUp.11",
						   "outputRampUp.12"};

const char *S_outputRampDown[12] ={"outputRampDown.1",
							 "outputRampDown.2",
						     "outputRampDown.3",
						     "outputRampDown.4",
						     "outputRampDown.5",
						     "outputRampDown.6",
						     "outputRampDown.7",
						     "outputRampDown.8",
						     "outputRampDown.9",
						     "outputRampDown.10",
						     "outputRampDown.11",
						     "outputRampDown.12"};

const char *S_outputStatus[12] = {"outputStatus.1",
							"outputStatus.2",
							"outputStatus.3",
							"outputStatus.4",
							"outputStatus.5",
							"outputStatus.6",
							"outputStatus.7",
							"outputStatus.8",
							"outputStatus.9",
							"outputStatus.10",
							"outputStatus.11",
							"outputStatus.12"};

const char *S_outputSwitch[12] = {"outputSwitch.1",
							"outputSwitch.2",
							"outputSwitch.3",
							"outputSwitch.4",
							"outputSwitch.5",
							"outputSwitch.6",
							"outputSwitch.7",
							"outputSwitch.8",
							"outputSwitch.9",
							"outputSwitch.10",
							"outputSwitch.11",
							"outputSwitch.12"};

oid oidSysMainSwitch [MAX_OID_LEN];
size_t lengthSysMainSwitch;
oid oidOutputMeasurementSenseVoltageAll [MAX_OID_LEN];
size_t lengthOutputMeasurementSenseVoltageAll;
oid oidFanAirTemperature[MAX_OID_LEN];
size_t lengthFanAirTemperature;
oid oidSensorTemperature  [8] [MAX_OID_LEN];
size_t lengthSensorTemperature[8];
oid oidOutputMeasurementSenseVoltage[12] [MAX_OID_LEN];
size_t lengthOutputMeasurementSenseVoltage[12];
oid oidOutputVoltage[12] [MAX_OID_LEN];
size_t lengthOutputVoltage[12];
oid oidOutputMeasurementCurrent[12] [MAX_OID_LEN];
size_t lengthOutputMeasurementCurrent[12];
oid oidOutputRampUp[12] [MAX_OID_LEN];
size_t lengthOutputRampUp[12];
oid oidOutputRampDown[12] [MAX_OID_LEN];
size_t lengthOutputRampDown[12];
oid oidOutputStatus[12] [MAX_OID_LEN];
size_t lengthOutputStatus[12];
oid oidChannelSwitch[12] [MAX_OID_LEN];
size_t lengthChannelSwitch[12];



// DLL functions

void syslog(int priority,const char* format,...);
int SnmpInit(void);
void SnmpCleanup(void);
HSNMP SnmpOpen(const char* ipAddress);
void SnmpClose(HSNMP m_sessp);
double getOutputVoltage(HSNMP m_sessp,int channel);
double setOutputVoltage(HSNMP m_sessp,int channel,double value);
double getCurrentMeasurement(HSNMP m_sessp, int channel);
double getOutputRampUp(HSNMP m_sessp,int channel);
double setOutputRampUp(HSNMP m_sessp,int channel,double value);
double getOutputRampDown(HSNMP m_sessp,int channel);
double setOutputRampDown(HSNMP m_sessp,int channel,double value);
int getMainSwitch(HSNMP m_sessp);
double setMainSwitch(HSNMP m_sessp, float value);
int getChannelStatus(HSNMP m_sessmp, int channel);
double setChannelSwitch(HSNMP m_sessmp, int channel, float value);
int getFantrayTemp(HSNMP m_sessp);
int getSensorTemp(HSNMP m_sessp, int sensor);
double getAllOutputSenseMeasurement(HSNMP m_sessp, double values[]);
