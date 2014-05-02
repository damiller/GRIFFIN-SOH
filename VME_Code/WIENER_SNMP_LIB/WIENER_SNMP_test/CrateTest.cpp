/** Test Program to Demonstrate C-Access to WIENER-Crates via SNMP
first test 04/23/06 running with DLL*/

#include "WIENER_SNMP.h"


 /* double power = getMainSwitch(crate1);
  printf("Power Status is %f\n", power);
  
  power = 1;
  
  power = setMainSwitch(crate1,power);
  printf("Power Status is now %f\n",power);	

  double u0 = getOutputVoltage(crate1,0);    // read U0 sense voltage
  printf("U0: Read Output Voltage U0: %f V\n",u0);

  for(;;) {
    printf("Enter new value (-1 to stop): "); scanf("%lf",&u0);
    if(u0 == -1.) break;
    u0 = setOutputVoltage(crate1,0,u0);
    printf("U0: Read Output Voltage U0: %f V\n",u0);
  }*/



char *ip(char * machine);

/******************************************************************************/
// Main.

int main(int argc, char* argv[])
{
  
  char *address;
  char action[77];
  bool bool1 = 1;
  double power=-1.0;

   
  if(!SnmpInit()) 
    {
    printf("Initialization Failed\n");
    power =  -5;                     // basic init
    } 
  else 
    {
      if(argc == 1) 
	{
	printf("Please Enter the Name of A Machine\n");
	power = -4;
	} 
      else 
	{
	  address = ip(argv[1]);
	  if(strcmp(address, "0") == 0)
	    {
	      printf("%s is not found in my local lookup table\n",argv[1]);
	      power = -2.0;
	    }
	  else
	    {
	      HSNMP crate1 = SnmpOpen(address);   // open TCP/IP socket
	      if(!crate1) 
		{
		  printf("Error opening address %s\n",address);
		  power=-3.0;
		}
	      else
		{
		  if(argc == 3)
		    {
		      strcpy(action, argv[2]);
		      bool1 = strcmp( action, "PowerOn");
		      if( bool1 == 0)
			{
			  power = 1;		
			  setMainSwitch(crate1,power);
			  printf("PowerOn");
			}
		      bool1 = strcmp( action, "PowerOff");
		      if( bool1 == 0)
			{
			  power = 0;
			  setMainSwitch(crate1,power);
			  printf("PowerOff");
			}
		    }
		/* Always check power as the last thing you do */
		power = (double) getMainSwitch(crate1);
		printf("...%s (%s) power status is %3.0f\n",
		       argv[1], address, power);
		SnmpClose(crate1);
		}
	    }
	}
    }
  SnmpCleanup();                              // finish
  return (int)power;
}

char* ip( char * machine)
{
   static char ip[100] = "0";

   sprintf(ip,"0");
   if(strcmp( machine,"tigvps01")==0) sprintf(ip, "142.90.102.107");
   if(strcmp( machine,"tigvps02")==0) sprintf(ip, "142.90.102.108");
   if(strcmp( machine,"tigvps03")==0) sprintf(ip, "142.90.102.109");
   if(strcmp( machine,"tigvps04")==0) sprintf(ip, "142.90.102.124");
   if(strcmp( machine,"tigxps01")==0) sprintf(ip, "142.90.102.101");
   if(strcmp( machine,"tigxps02")==0) sprintf(ip, "142.90.102.102");
   if(strcmp( machine,"tigxps03")==0) sprintf(ip, "142.90.102.103");
   if(strcmp( machine,"tigxps04")==0) sprintf(ip, "142.90.102.104");
   if(strcmp( machine,"tigxps05")==0) sprintf(ip, "142.90.102.105");
   if(strcmp( machine,"tigxps06")==0) sprintf(ip, "142.90.102.106");
   if(strcmp( machine,"tigxps07")==0) sprintf(ip, "142.90.102.117");
   if(strcmp( machine,"tigxps08")==0) sprintf(ip, "142.90.102.118");
   if(strcmp( machine,"tigxps09")==0) sprintf(ip, "142.90.102.119");
   if(strcmp( machine,"tigxps10")==0) sprintf(ip, "142.90.102.120");
   if(strcmp( machine,"tigxps11")==0) sprintf(ip, "142.90.102.121");

   return ip;
}

