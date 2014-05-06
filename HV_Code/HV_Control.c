//INCLUDE FILES
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <termios.h>
#include <stdbool.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "caenhvoslib.h"
#include "CAENHVWrapper.h"


/*****************************************************************************/

//GLOBAL VARIABLE DEFINITIONS

//unit key array (for outputting the units of various properties) -from demo code
static char *ParamUnitStr[] = {
   "None",
   "Amperes",
   "Volts",
   "Watts",
   "Celsius",
   "Hertz",
   "Bar",
   "Volt/sec",
   "sec",
   "rpm",                       // Rel. 2.0. - Linux
   "counts"
};                              // Rel. 2.6

//structure called ParProp which holds the properties of the parameters of channels and boards (from demo code)
typedef struct ParPropTag {
   unsigned long Type, Mode;
   float Minval, Maxval;
   unsigned short Unit;
   short Exp;
   char Onstate[30], Offstate[30];
} ParProp;

//HV Power Supply Connection Type
const int LinkType = LINKTYPE_TCPIP;

//Default User name, password
char UserName[20] = "Incorrect";
char Password[20] = "Incorrect";

//Structures for echo on/off function
static struct termios default_settings;
static int settings_changed = 0;

//Other
char System[77] = "Undefined";
int SlotMap[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int SystemHandle = 0;

unsigned short NrOfSlot, NrSlot;
unsigned short NrOfChList[50];

/*****************************************************************************/

//FUNCTION DEFINITIONS

int UserLogin(int systemNumber);
int System_Init(const char *hostname);
int System_Deinit(void);
int System_Prop(int systemNumber);
int Crate_Map();
int Brd_Prop();
int Ch_Prop();
int ParProp_Read(int chOrBrd, int relSlot, unsigned short slot,
                 unsigned short ch, char *parameter, ParProp * property);
int Comm_Control();
int Kill_All();
int PowerToggle();
int Power_All();
int Power(unsigned setting);
int SetPar(int slot, char *parameter, unsigned short chNum,
           const unsigned short *chList, void *val);
int Voltage();
void set_echo(int setval);


/*****************************************************************************/
//MAIN FUNCTION                                                               /
/*****************************************************************************/

main(int argc, char *argv[])
{
   //Local Variables
   int systemNumber = 4;
   int returnCode = 1;
   int anything;
   char num[1];
   char action[77];
   int bool1 = 1;

   /* first things first */
   // caenhvwrapper_init();
   if (argc == 3) {
      strcpy(UserName, "admin");
      strcpy(Password, "admin");

      strcpy(System, argv[1]);
      systemNumber = atoi(num);

      returnCode = System_Init(System);

      strcpy(action, argv[2]);

      bool1 = strcmp(action, "KillAll");
      if (bool1 == 0) {
         returnCode = Kill_All();
      };

      bool1 = strcmp(action, "PowerToggle");
      if (bool1 == 0) {
         returnCode = PowerToggle();
      };

      bool1 = strcmp(action, "PowerOn");
      if (bool1 == 0) {
         returnCode = Power(1);
      };

      bool1 = strcmp(action, "PowerOff");
      if (bool1 == 0) {
         returnCode = Power(0);
      };

      bool1 = strcmp(action, "SetVoltage");
      if (bool1 == 0) {
         returnCode = Voltage();
      };

      bool1 = strcmp(action, "Test");
      if (bool1 == 0) {
         returnCode = Test_Program();
      };

      bool1 = strcmp(action, "PowerAll");
      if (bool1 == 0) {
         returnCode = Power_All();
      };
   }

   else {
      if (argc == 1) {
       login:                  //user login
         returnCode = UserLogin(systemNumber);
         if (returnCode) {
            fprintf(stderr, "Login Failed: %s\n");
            goto login;
         };
      }

      else if (argc == 2) {
         strcpy(UserName, "admin");
         strcpy(Password, "admin");

         strcpy(System, argv[1]);
      }
      //initialize system   
      returnCode = System_Init(System);
      if (returnCode) {
         fprintf(stderr, "Initialization Failed: %s\n",
                 CAENHV_GetError(SystemHandle));
      }

      else {
       read:                   //get system properties
         returnCode = System_Prop(systemNumber);
         if (returnCode) {
            fprintf(stderr, "Cannot print out system properties.\n");
         };

         //get crate map
         returnCode = Crate_Map();
         if (returnCode) {
            fprintf(stderr, "Cannot print out Crate Map.\n");
         };

         //get board parameter properties
         returnCode = Brd_Prop();
         if (returnCode) {
            fprintf(stderr, "Cannot print out Board Properties.\n");
         };

         //get channel parameter properties
         returnCode = Ch_Prop();
         if (returnCode) {
            fprintf(stderr, "Cannot print out Channel Properties.\n");
         };

         //Perform an action on the system
         returnCode = Comm_Control();
         if (returnCode) {
            goto read;
         };

         //deinitialize system
         returnCode = System_Deinit();
         if (returnCode) {
            fprintf(stderr, "Deinitialization Failed: %s\n",
                    CAENHV_GetError(SystemHandle));
         };
      };
   }
   // caenhvwrapper_fini();
}


/*****************************************************************************/
//USER VERIFICATION FUNCTION                                                  /
/*****************************************************************************/
//This function verifies the username and password typed in by the user

int UserLogin(systemNumber)
{
   //Local Variables
   int bool1 = 1;
   int bool2 = 1;

   while (bool1 | bool2) {
      fprintf(stderr, "Username: ");
      scanf("%s", UserName);
      fprintf(stderr, "Password: ");
      set_echo(0);
      scanf("%s", Password);
      set_echo(1);
      fprintf(stderr, "\n");
      fflush(stdin);
      
      bool1 = strcmp(UserName, "admin");
      bool2 = strcmp(Password, "admin");

      if (bool1 | bool2) {
	 fprintf(stderr, "Invalid Entry. ");
      };
   }

   return 0;
};

/*****************************************************************************/
//SYSTEM INITIALIZATION FUNCTION                                              /
/*****************************************************************************/
//This function initializes the system

int System_Init(const char *hostname)
{
   char ipString[20];

   CAENHVRESULT returnCode = 0;
   // do the hostname lookup
   struct hostent *entity = gethostbyname(hostname);
   if (entity == NULL) {
      printf("Host %s not found.\n", hostname);
      return CAENHV_COMMUNICATIONERROR;
   }      
   if (entity->h_addrtype != AF_INET) {
      printf("Wrong address family.\n");
      return CAENHV_COMMUNICATIONERROR;
   }

   struct in_addr *addr = (struct in_addr*)entity->h_addr_list[0];  
   returnCode = CAENHV_InitSystem(SY1527, LinkType, 
				  (void *) inet_ntoa(*addr),
				  (const char *) UserName,
				  (const char *) Password, 
				  &SystemHandle);
   return returnCode;
};


/*****************************************************************************/
//SYSTEM DEINITIALIZATION FUNCTION                                            /
/*****************************************************************************/
//This function deinitializes the system

int System_Deinit(void)
{
   return CAENHV_DeinitSystem(SystemHandle);
};


/*****************************************************************************/
//SYSTEM PROPERTY OUTPUT FUNCTION                                             /
/*****************************************************************************/
//This function outputs all of the system properties

int System_Prop(systemNumber)
{
//Local Variables

   //for the list of system properties
   unsigned short numProp = 20;
   char propName[1024];
   char *namePtr;
   char *namePtrStart;

   //for the values of the system properties
   union {
      char cRes[4096];
      float fRes;
      unsigned short usRes;
      unsigned long ulRes;
      short sRes;
      long lRes;
      unsigned uRes;
   } result;

   //for the information on the system properties
   unsigned propMode;
   unsigned propType;

   //other
   int i = 0;
   int returnCode = 0;

   namePtr = propName;          //initializes the name pointer

   returnCode = CAENHV_GetSysPropList(SystemHandle, &numProp, &namePtr);

   if (returnCode)              //If there is an error in executing the property list function, prints out the error and returns to the main function with an error code
   {
      fprintf(stderr, "PropList ERROR: %s   (%d).  ",
              CAENHV_GetError(SystemHandle), returnCode);
      return 1;
   };

   fprintf(stdout, "--------------------------------------------------------------------------------\n");       //starts output list 
   fprintf(stdout, "SYSTEM PROPERTIES OF %s\n", System);
   fprintf(stdout,
           "--------------------------------------------------------------------------------\n");
   fprintf(stdout, "Property    Mode Type  Value\n");
   fprintf(stdout,
           "--------------------------------------------------------------------------------\n");

   namePtrStart = namePtr;

   for (i = 0; i < numProp; i++, namePtr += (strlen(namePtr) + 1))      //Output loop for the system properties and their types, modes, and values
   {
      fprintf(stdout, "%-14s", namePtr);        //Prints out the system property names

      //Gets the system property mode and type
      returnCode = CAENHV_GetSysPropInfo(SystemHandle, (const char *) namePtr,
					 &propMode, &propType);

      if (returnCode)           //If there is an error in executing the property info function, prints out the error and skips to end of for loop
      {
         fprintf(stdout, "PropInfo ERROR: %s   (%d)\n",
                 CAENHV_GetError(SystemHandle), returnCode);
         goto skip;
      };

      fprintf(stdout, "%-5u", propMode);        //prints out property mode 
      fprintf(stdout, "%-4u", propType);        //prints out property type

      //Gets the system property value
      returnCode = CAENHV_GetSysProp(SystemHandle, (const char *) namePtr, &result);    

      if (returnCode)           //If there is an error in executing the property value function, prints out the error
      {
         fprintf(stdout, "PropValue ERROR: %s   (%d)\n",
                 CAENHV_GetError(SystemHandle), returnCode);
      }

      else                      //No error => print out the value of the property
      {
         switch (propType) {
         case SYSPROP_TYPE_STR:
            {
               fprintf(stdout, "%-20s\n", result.cRes);
               break;
            };
         case SYSPROP_TYPE_REAL:
            {
               fprintf(stdout, "%-20f\n", result.fRes);
               break;
            };
         case SYSPROP_TYPE_UINT2:
            {
               fprintf(stdout, "%-20u\n", result.usRes);
               break;
            };
         case SYSPROP_TYPE_UINT4:
            {
               fprintf(stdout, "%-20u\n", result.ulRes);
               break;
            };
         case SYSPROP_TYPE_INT2:
            {
               fprintf(stdout, "%-20d\n", result.sRes);
               break;
            };
         case SYSPROP_TYPE_INT4:
            {
               fprintf(stdout, "%-20d\n", result.lRes);
               break;
            };
         case SYSPROP_TYPE_BOOLEAN:
            {
               fprintf(stdout, "%-20u\n", result.uRes);
               break;
            };
         };                     // end of switch
      };                        // end of else
    skip:                      //if there is an error in obtaining property info, program skips to the end of the for loop, bypassing incompletable output steps
      returnCode = 0;           //make compiler happy with use of label (skip:) at end of compound statement
   };                           // end of for

   free(namePtrStart);          //deallocates memory in the propName array before returning to main function with no error
   fprintf(stdout,
           "--------------------------------------------------------------------------------\n\n\n");

   return 0;

};


/*****************************************************************************/
//CRATE MAP OUTPUT FUNCTION                                                   /
/*****************************************************************************/
//This function outputs the crate map for the system.

int Crate_Map()
{
   //Local Variables

   //Get Crate Map function parameters

   unsigned short *chListPtr;
   unsigned short *chListPtrStart;
   char modelList[1024];
   char *modelListPtr;
   char *modelListPtrStart;
   char descriptionList[1024];
   char *descriptionListPtr;
   char *descriptionListPtrStart;
   unsigned short serNumList[50];
   unsigned short *serListPtr;
   unsigned short *serListPtrStart;
   unsigned char fmwRelMinList[50];
   unsigned char *minListPtr;
   unsigned char *minListPtrStart;
   unsigned char fmwRelMaxList[50];
   unsigned char *maxListPtr;
   unsigned char *maxListPtrStart;

   //Other
   int returnCode = 0;
   int i = 0, j = 0;

   //initialize the pointers to pass into the Get Crate Map function
   chListPtr = NrOfChList;
   modelListPtr = modelList;
   descriptionListPtr = descriptionList;
   serListPtr = serNumList;
   minListPtr = fmwRelMinList;
   maxListPtr = fmwRelMaxList;

   //get the crate map
   returnCode =
       CAENHV_GetCrateMap(SystemHandle, &NrOfSlot, &chListPtr,
                         &modelListPtr, &descriptionListPtr, &serListPtr,
                         &minListPtr, &maxListPtr);

   if (returnCode)              //if there is an error in obtaining the crate map, outputs the error and returns to main with an error code
   {
      fprintf(stdout, "CrateMap ERROR: %s   (%d).  ",
              CAENHV_GetError(SystemHandle), returnCode);
      return 1;
   };

   //initialize the "placeholder" pointers for later freeing the allocated memory
   chListPtrStart = chListPtr;
   modelListPtrStart = modelListPtr;
   descriptionListPtrStart = descriptionListPtr;
   serListPtrStart = serListPtr;
   minListPtrStart = minListPtr;
   maxListPtrStart = maxListPtr;

   fprintf(stdout, "--------------------------------------------------------------------------------\n");       //starts output list 
   fprintf(stdout, "CRATE MAP FOR %s\n", System);
   fprintf(stdout,
           "--------------------------------------------------------------------------------\n");
   fprintf(stdout,
           "Slot#  # Ch.  Brd Model         Description          Serial #  Firmware Release\n");
   fprintf(stdout,
           "--------------------------------------------------------------------------------\n");


   //Output loop for the crate map
   for (i = 0; i < NrOfSlot;
        i++, modelListPtr +=
        (strlen(modelListPtr) + 1), descriptionListPtr +=
        (strlen(descriptionListPtr) + 1), minListPtr +=
        sizeof(unsigned char), maxListPtr += sizeof(unsigned char)) {
      fprintf(stdout, "  %-7d", i);
      fprintf(stdout, "%-6u %-9s %-28s %-13u %-2c.%-2c\n", chListPtr[i],
              modelListPtr, descriptionListPtr, serListPtr[i], *maxListPtr,
              *minListPtr);
   };                           //end of for

   fprintf(stdout,
           "--------------------------------------------------------------------------------\n\n\n");

   for (i = 0, j = 0; i < NrOfSlot; i++) {
      NrOfChList[i] = chListPtr[i];
      if (chListPtr[i]) {
         SlotMap[j] = i;
         j++;
      };                        //end of if
      NrSlot = j;
   };                           //end of for

   //deallocate memory before returning to main function with no error
   free(chListPtrStart);
   free(modelListPtrStart);
   free(serListPtrStart);
   free(minListPtrStart);
   free(descriptionListPtrStart);
   free(maxListPtrStart);

   return 0;
};


/*****************************************************************************/
//BOARD OUTPUT FUNCTION                                                       /
/*****************************************************************************/
//This functions outputs all of the information pertaining to the boards

int Brd_Prop()
{
   //Local Variables
   //for Get Board Param Info function
   unsigned short slot;
   //  char paramName[1024];
   char *paramNameList = (char *) NULL;
   char *paramNamePtrStart;
   char (*parameter)[MAX_PARAM_NAME];

   //for Get Board Param Prop function
   ParProp *property;
   int numPar = 0;

   //other
   int i = 0, j = 0;
   int returnCode = 1;

   do                           //do-while loop so that the function outputs one list of board parameter properties for each used slot in the crate
   {
      paramNameList = (char *) NULL;
      property = NULL;

      slot = SlotMap[i];

      returnCode = CAENHV_GetBdParamInfo(SystemHandle, slot, &paramNameList);

      if (returnCode)           //if there is an error in obtaining the parameter names, outputs the error and skips to end of loop for this slot
      {
         fprintf(stderr, "BrdParam ERROR (slot %d): %s   (%d).  ", slot,
                 CAENHV_GetError(SystemHandle), returnCode);
         goto next;
      };

      paramNamePtrStart = paramNameList;

      parameter = (char (*)[MAX_PARAM_NAME]) paramNameList;

      for (j = 0; parameter[j][0]; j++);
      numPar = j;

      property = calloc(numPar, sizeof(ParProp));

      fprintf(stdout, "--------------------------------------------------------------------------------\n");    //starts output list 
      fprintf(stdout, "BOARD PARAMETER PROPERTIES FOR %s, SLOT %d\n",
              System, SlotMap[i]);
      fprintf(stdout,
              "--------------------------------------------------------------------------------\n");

      for (j = 0; j < numPar; j++) {
         returnCode =
             ParProp_Read(0, i, slot, 0, parameter[j], &(property[j]));
         if (returnCode) {
            fprintf(stdout, "\n");
         };
      };                        //end for (parameters)

    next:i++;
      free(property);
      free(paramNamePtrStart);  //frees memory pointed to by the parameter name pointer
   }                            //end do
   while (SlotMap[i]);

   fprintf(stdout,
           "--------------------------------------------------------------------------------\n\n\n");

   return 0;
};



/*****************************************************************************/
//CHANNEL READOUT FUNCTION                                                    /
/*****************************************************************************/
//this function outputs all of information pertaining to the channels

int Ch_Prop()
{
   //Local Variables
   //for get channel parameter name function
   unsigned short slot;
   unsigned short chNum;
   int parNum = 0;

   //for get channle parameter property function
   ParProp *property;

   //other
   int i = 0, k = 0, returnCode;
   unsigned short j = 0;

   do {
      char (*chNameList)[MAX_CH_NAME] = NULL;
      slot = SlotMap[i];
      chNum = NrOfChList[slot];

      unsigned short *chList = NULL;
      chList = malloc(chNum * sizeof(unsigned short));

      for (j = 0; j < chNum; j++) {
         chList[j] = j;
      };

      chNameList = malloc(chNum * MAX_CH_NAME * sizeof(char));

      returnCode = CAENHV_GetChName(SystemHandle, slot, 
				    chNum, chList, chNameList);
      if (returnCode)           //if there is an error in obtaining the name list, outputs the error and skips to the next slot
      {
         fprintf(stderr, "ChName ERROR (slot %d): %s   (%d).", slot,
                 CAENHV_GetError(SystemHandle), returnCode);
         goto ns;
      };

      for (j = 0; j < chNum; j++) {
         char *parNameList = (char *) NULL, *parNamePtr;
	 int parNumber;
         //      parNameList = malloc(20*MAX_PAR_NAME*sizeof(char));


         fprintf(stdout, "--------------------------------------------------------------------------------\n"); //starts output list 
         fprintf(stdout,
                 "CHANNEL PARAMETER PROPERTIES FOR %s, SLOT %d, Channel %d (%s)\n",
                 System, slot, chList[j], chNameList[j]);
         fprintf(stdout, "    Name      Parameter Value\n");
         fprintf(stdout,
                 "--------------------------------------------------------------------------------\n");

         returnCode = CAENHV_GetChParamInfo(SystemHandle, slot, j,
					    &parNameList, &parNum);
         if (returnCode)        //if there is an error in obtaining the name list, outputs the error and skips to the next channel
         {
            fprintf(stderr, "ParName ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            goto nc;
         };

         property = calloc(parNum, sizeof(ParProp));

         for (k = 0, parNamePtr = parNameList; k < parNum;
              k++, parNamePtr += (10)) {
            returnCode =
	       ParProp_Read(1, chNum, slot, j, parNamePtr,
			    &(property[k]));
            if (returnCode)     //if there is an error in obtaining the parameter, outputs the error and skips to the next parameter
            {
               fprintf(stderr, "ParProp ERROR (slot %d): %s   (%d).", slot,
                       CAENHV_GetError(SystemHandle), returnCode);
               goto np;
            };

            //              fprintf (stdout, "%-5d %-5d", property[k].Type, property[k].Mode);

          np:fprintf(stdout, "\n");
            /* fprintf (stdout, "--------------------------------------------------------------------------------\n");   */
         };                     //end for (parameters)

       nc:free(parNameList);
         parNameList = NULL;
         free(property);
      };                        //end for (channels)

      free(chNameList);
      free(chList);

    ns:i++;
   }                            //end do (slots)
   while (SlotMap[i]);


   return 0;
};




/*****************************************************************************/
//GET PARAMETER PROPERTY FUNCTION                                             /
/*****************************************************************************/
//This function gets and outputs the parameter properties for a channel or board

int ParProp_Read(int chOrBrd, int relSlot, unsigned short slot,
                 unsigned short ch, char *parameter, ParProp * property)
{
   //Local Variables
   int returnCode = 1, i = 0;
   unsigned short j = 0;
   float pvlf[256];
   unsigned long pvlul[256];
   unsigned short pvlus[256];

   fprintf(stdout, "%-12s:", parameter);
   fflush(stdout);

   if (chOrBrd == 0)            //Reads and outputs parameters for the BOARD
   {
      //First get property parameter mode and type in order to later retrieve the property values
      //Gets property type
      returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter, 
					 "Type", &(property->Type));
      if (returnCode)           //if there is an error in obtaining the parameter type, outputs the error and skips to end of loop for this parameter
      {
         fprintf(stdout, " BrdProp ERROR (slot %d): %s   (%d).", slot,
                 CAENHV_GetError(SystemHandle), returnCode);
         return 1;
      };
      //Gets property mode
      returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					 "Mode", &(property->Mode));
      if (returnCode)           //if there is an error in obtaining the parameter mode, outputs the error and skips to end of loop for this parameter
      {
         fprintf(stdout, " BrdProp ERROR (slot %d): %s   (%d).", slot,
                 CAENHV_GetError(SystemHandle), returnCode);
         return 1;
      };


      //Next, we want to output the actual values of the parameters
      if ((property->Type) == PARAM_TYPE_NUMERIC)       //If the type is numeric, outputs the value
      {
         float *parValList = pvlf;

         returnCode = CAENHV_GetBdParam(SystemHandle, NrSlot,
                              (const unsigned short *) SlotMap, parameter,
                              parValList);
         if (returnCode)        //if there is an error in obtaining the parameter, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, " BrdVal ERROR (%s): %s   (%d).", parameter,
                    CAENHV_GetError(SystemHandle), returnCode);
            goto next;
            free(parValList);
            parValList = NULL;
         };
         fprintf(stdout, " %g", parValList[relSlot]);
      };

      if ((property->Type) == PARAM_TYPE_ONOFF) //If the type is on/off, outputs the value
      {
         unsigned short *parValList = pvlus;
         returnCode = CAENHV_GetBdParam(SystemHandle, NrSlot,
                              (const unsigned short *) SlotMap, parameter,
                              parValList);
         if (returnCode)        //if there is an error in obtaining the parameter, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, " BrdVal ERROR (%s): %s   (%d).", parameter,
                    CAENHV_GetError(SystemHandle), returnCode);
            free(parValList);
            parValList = NULL;
            goto next;
         };
         if (parValList[relSlot] == 0) {
            fprintf(stdout, " Off");
         };
         if (parValList[relSlot] == 1) {
            fprintf(stdout, " On");
         };
      };

      if ((property->Type) == PARAM_TYPE_BDSTATUS)      //If the type is board status, retrieves and outputs the status
      {
         unsigned long *parValList = pvlul;
         returnCode = CAENHV_GetBdParam(SystemHandle, NrSlot,
                              (const unsigned short *) SlotMap, parameter,
                              parValList);
         if (returnCode)        //if there is an error in obtaining the parameter, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, " BrdVal ERROR (%s): %s   (%d).", parameter,
                    CAENHV_GetError(SystemHandle), returnCode);
            free(parValList);
            parValList = NULL;
            goto next;
         };

         if (parValList[relSlot])       //If there is an error in the board status, outputs that error
         {
            if (parValList[relSlot] & 1) {
               fprintf(stdout, " Power-Fail!");
            };
            if (parValList[relSlot] & 2) {
               fprintf(stdout, " Firmware Checksum Error!");
            };
            if (parValList[relSlot] & 4) {
               fprintf(stdout, " HV Callibration Error!");
            };
            if (parValList[relSlot] & 8) {
               fprintf(stdout, " Temperature Callibration Error!");
            };
            if (parValList[relSlot] & 16) {
               fprintf(stdout, " Under-Temperature!");
            };
            if (parValList[relSlot] & 32) {
               fprintf(stdout, " Over-Temperature!");
            };
            goto next;
         };

         fprintf(stdout, " Board OK");  //Otherwise outputs the status as ok
      };

    next:
      fprintf(stdout, "\n              ");

      //retrieve and output property values given their type
      if ((property->Type) == PARAM_TYPE_NUMERIC)       //If the type is numeric, retrieves and outputs the numeric properties for the parameter
      {
         returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					    "Minval", &(property->Minval));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, "BrdProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };

         returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					    "Maxval", &(property->Maxval));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, "BrdProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, "Min: %-7g Max: %-7g", property->Minval,
                 property->Maxval);

         returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					    "Exp", &(property->Exp));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, "BrdProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, " (e.%-2d ", property->Exp);

         returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					    "Unit", &(property->Unit));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, "BrdProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, "%s)", ParamUnitStr[property->Unit]);
      };


      if ((property->Type) == PARAM_TYPE_ONOFF) //If the type is On/Off, retrieves and outputs the onstate and offstate
      {
         returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					    "Onstate", &(property->Onstate));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, "BrdProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, "%-10s", property->Onstate);

         returnCode = CAENHV_GetBdParamProp(SystemHandle, slot, parameter,
					    "Offstate", &(property->Offstate));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, "BrdProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, "%-10d\n", property->Offstate);
      };

      fprintf(stdout, "\n");
      return 0;
   };                           //end if

   if (chOrBrd == 1)            //gets and outputs the parameters and properties for the CHANNEL
   {
      //first get property parameter mode and type in order to later retrieve the property values
      //Gets property type
      returnCode = CAENHV_GetChParamProp(SystemHandle, slot, ch, parameter,
					 "Type", &(property->Type));        
      if (returnCode)           //if there is an error in obtaining the parameter type, outputs the error and skips to end of loop for this parameter
      {
         fprintf(stdout, "ChProp ERROR (slot %d): %s   (%d).", slot,
                 CAENHV_GetError(SystemHandle), returnCode);
         return 1;
      };

      //Gets property mode
      returnCode = CAENHV_GetChParamProp(SystemHandle, slot, ch, parameter,
					 "Mode", &(property->Mode));       
      if (returnCode)           //if there is an error in obtaining the parameter mode, outputs the error and skips to end of loop for this parameter
      {
         fprintf(stdout, "ChProp ERROR (slot %d): %s   (%d).", slot,
                 CAENHV_GetError(SystemHandle), returnCode);
         return 1;
      };

      //Next, we want to output the actual values of the parameters

      //make a list of two channels to pass into the parameter value function
      unsigned short *chList = NULL;
      chList = malloc(1 * sizeof(unsigned short));

      chList[0] = ch;

      if ((property->Type) == PARAM_TYPE_NUMERIC)       //If the type is numeric, outputs the numeric value
      {
         float *parValList = NULL;
         parValList = malloc(1 * sizeof(float));

         returnCode = CAENHV_GetChParam(SystemHandle, slot, parameter,
					1, (const unsigned short *) chList,
					parValList);
         if (returnCode)        //if there is an error in obtaining the parameter, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, " ChVal ERROR (%s): %s   (%d).", parameter,
                    CAENHV_GetError(SystemHandle), returnCode);
            free(parValList);
            parValList = NULL;
            goto next1;
         };
         fprintf(stdout, " %g", parValList[0]);
         free(parValList);
         parValList = NULL;
      };

      if ((property->Type) == PARAM_TYPE_ONOFF) //If the type is on/off, outputs on or off
      {
         unsigned short *parValList = NULL;
         parValList = malloc(1 * sizeof(unsigned short));

         returnCode = CAENHV_GetChParam(SystemHandle, slot, parameter,
					1, (const unsigned short *) chList,
					(unsigned short *) parValList);
         if (returnCode)        //if there is an error in obtaining the parameter, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, " ChVal ERROR (%s): %s   (%d).", parameter,
                    CAENHV_GetError(SystemHandle), returnCode);
            free(parValList);
            parValList = NULL;
            goto next1;
         };
         if (parValList[0] == 0) {
            fprintf(stdout, " Off");
         } else {
            fprintf(stdout, " On");
         };
         free(parValList);
         parValList = NULL;
      };

      if ((property->Type) == PARAM_TYPE_CHSTATUS)      //If the type is channel status, retrieves and outputs the status
      {
         unsigned long *parValList = NULL;
         parValList = malloc(1 * sizeof(unsigned long));

         returnCode = CAENHV_GetChParam(SystemHandle, slot, parameter,
					1, (const unsigned short *) chList,
					parValList);
         if (returnCode)        //if there is an error in obtaining the parameter, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stdout, " ChVal ERROR (%s): %s   (%d).", parameter,
                    CAENHV_GetError(SystemHandle), returnCode);
            free(parValList);
            parValList = NULL;
            goto next1;
         };

         if (parValList[0])     //Outputs te board's status if one is returned
         {
            if (parValList[0] & 1) {
               fprintf(stdout, " Channel Is On");
            };
            if (parValList[0] & 2) {
               fprintf(stdout, " Channel is ramping up");
            };
            if (parValList[0] & 4) {
               fprintf(stdout, " Channel is ramping down");
            };
            if (parValList[0] & 8) {
               fprintf(stdout, " Channel is in overcurrent!");
            };
            if (parValList[0] & 16) {
               fprintf(stdout, " Channel is in overvoltage!");
            };
            if (parValList[0] & 32) {
               fprintf(stdout, " Channel is in undervoltage!");
            };
            if (parValList[0] & 64) {
               fprintf(stdout, " Channel is in external trip!");
            };
            if (parValList[0] & 128) {
               fprintf(stdout, " Channel is in max V!");
            };
            if (parValList[0] & 256) {
               fprintf(stdout, " Channel is in external disable!");
            };
            if (parValList[0] & 512) {
               fprintf(stdout, " Channel is in internal trip!");
            };
            if (parValList[0] & 1024) {
               fprintf(stdout, " Channel is in callibration error!");
            };
            if (parValList[0] & 2048) {
               fprintf(stdout, " Channel is unplugged!");
            };
            free(parValList);
            parValList = NULL;
            goto next1;
         };

         fprintf(stdout, " No Info (Channel is off)");  //Otherwise outputs the status as ok
         free(parValList);
         parValList = NULL;
      };

    next1:
      free(chList);
      /* fprintf (stdout, "\n              "); */


      // PARAMETER PROPERTIES (not use while debugging)
#if (PROP)

      //retrieve and output property values given their type
      if ((property->Type) == PARAM_TYPE_NUMERIC)       //If the type is numeric, retrieves and outputs the numeric properties for the parameter
      {
         returnCode =
             CAENHVGetChParamProp(System, slot, ch, parameter, "Minval",
                                  &(property->Minval));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stderr, "ChProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHVGetError((const char *) System), returnCode);
            return 1;
         };

         returnCode =
             CAENHVGetChParamProp(System, slot, ch, parameter, "Maxval",
                                  &(property->Maxval));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stderr, "ChProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHVGetError((const char *) System), returnCode);
            return 1;
         };
         fprintf(stdout, "Min: %-7g Max: %-7g", property->Minval,
                 property->Maxval);

         returnCode =
             CAENHVGetChParamProp(System, slot, ch, parameter, "Exp",
                                  &(property->Exp));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stderr, "ChProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHVGetError((const char *) System), returnCode);
            return 1;
         };
         fprintf(stdout, " (e.%-2d ", property->Exp);

         returnCode =
             CAENHVGetChParamProp(System, slot, ch, parameter, "Unit",
                                  &(property->Unit));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stderr, "ChProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHVGetError((const char *) System), returnCode);
            return 1;
         };
         fprintf(stdout, "%s)", ParamUnitStr[property->Unit]);
      };


      if ((property->Type) == PARAM_TYPE_ONOFF) //If the type is On/Off, retrieves and outputs the onstate and offstate
      {
         returnCode =
             CAENHVGetChParamProp(System, slot, ch, parameter, "Onstate",
                                  &(property->Onstate));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stderr, "ChProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, "%-10s", property->Onstate);

         returnCode =
             CAENHVGetChParamProp(System, slot, ch, parameter, "Offstate",
                                  &(property->Offstate));
         if (returnCode)        //if there is an error in obtaining the property, outputs the error and skips to end of loop for this parameter
         {
            fprintf(stderr, "ChProp ERROR (slot %d): %s   (%d).", slot,
                    CAENHV_GetError(SystemHandle), returnCode);
            return 1;
         };
         fprintf(stdout, "%-10d", property->Offstate);
      };

      //end of PARAMETER PROPERTIES
#endif

      return 0;
   };

};

/*****************************************************************************/
//COMMAND CONTROL FUNCTION                                                    /
/*****************************************************************************/
//This function takes user input to determine the next course of action

int Comm_Control()
{
   int comNum = 0, returnCode = 1;
   char response = 'n';

 start:
   //First query the user on whether they would like to perform an action
   fprintf(stderr,
           "Would you like to execute a command or set a parameter on %s? (y or n)\n",
           System);
   fflush(stdout);
   fflush(stdin);
   scanf("%c", &response);

   switch (response) {
   case 'y':
      {
         break;
      };
   case 'n':
      {
         return 0;
      };
   default:
      {
         fprintf(stderr, "Invalid entry.\n");
         goto start;
      };
   };


 list:
   //List the possible actions that can be performed
   fprintf(stderr,
           "Please enter the command number you would like to execute from the list below:\n\n1. Re-readout all of the properties and parameters for the system\n2. Kill all channels in system\n3. Toggle the power status of channels\n4. Turn channel power on\n5. Turn channel power off\n6. Set the V0 value of channel(s)\n7. Run test program\n8. Turn power for all channels in a slot on\n0. None of the above\n\n");

   scanf("%d", &comNum);

   //Call the appropriate function depending on the chosen action
   switch (comNum) {
   case 0:
      {
         goto start;
      };
   case 1:
      {
         return 1;
      };
   case 2:
      {
         returnCode = Kill_All();
         if (returnCode) {
            fprintf(stderr, "Could not kill channels\n");
            goto start;
         };
         goto start;
      };
   case 3:
      {
         returnCode = PowerToggle();
         if (returnCode) {
            fprintf(stderr, "Could not toggle power\n");
            goto start;
         };
         goto start;
      };
   case 4:
      {
         returnCode = Power(1);
         if (returnCode) {
            fprintf(stderr, "Could not turn power on\n");
            goto start;
         };
         goto start;
      };
   case 5:
      {
         returnCode = Power(0);
         if (returnCode) {
            fprintf(stderr, "Could not turn power off\n");
            goto start;
         };
         goto start;
      };
   case 6:
      {
         returnCode = Voltage();
         if (returnCode) {
            fprintf(stderr, "Could not set voltage\n");
            goto start;
         };
         goto start;
      };
   case 7:
      {
         returnCode = Test_Program();
         if (returnCode) {
            fprintf(stderr, "Test not completed\n");
            goto start;
         };
         goto start;
      };
   case 8:
      {
         returnCode = Power_All();
         if (returnCode) {
            fprintf(stderr, "Could not turn power on\n");
            goto start;
         };
         goto start;
      };
   default:
      {
         fprintf(stderr, "Invalid entry.\n");
         goto list;
      };
   };                           //end switch
   return 0;
};



/*****************************************************************************/
//TURN OFF WHOLE CRATE FUNCTION                                               /
/*****************************************************************************/
//This function kills all of the channels in the crate

int Kill_All()
{
   int returnCode;

   returnCode = CAENHV_ExecComm(SystemHandle, "Kill");
   if (returnCode)              //If there is an error in killing the channels, prints out the error
   {
      fprintf(stderr, "ExecComm ERROR: %s   (%d): ",
              CAENHV_GetError(SystemHandle), returnCode);
   };
   return returnCode;
};


/*****************************************************************************/
//TOGGLE ON/OFF POWER STATUS OF A CHANNEL                                     /
/*****************************************************************************/
//This function turns a channel on if off and vice-versa

int PowerToggle()
{
   int returnCode = 1;
   int slot = 0, temp = 0, i = 0;
   unsigned short *channel = NULL, chNum = 0;
   unsigned pwVal;
   unsigned val;

   fprintf(stderr, "Please enter the slot number\n");
   scanf("%d", &temp);
   slot = temp;


   fprintf(stderr, "Please enter the number of channels to toggle\n");
   scanf("%d", &temp);
   chNum = (unsigned short) temp;

   channel = calloc(chNum, sizeof(unsigned short));

   fprintf(stderr,
           "Please enter the channel numbers, separated by spaces\n");

   for (i = 0; i < chNum; i++) {
      scanf("%d", &temp);
      channel[i] = (unsigned short) temp;
   };

   for (i = 0; i < chNum; i++) {
      returnCode = CAENHV_GetChParam(SystemHandle, slot, "Pw", 
				     1, &channel[i], (void *) &pwVal);
      if (returnCode) {
         fprintf(stderr, "GetParam ERROR: %s   (%d): ",
                 CAENHV_GetError(SystemHandle), returnCode);
         return 1;
      };

      switch (pwVal) {
      case 0:
         val = 1;
         break;
      case 1:
         val = 0;
         break;
      };                        //end switch

      returnCode = CAENHV_SetChParam(SystemHandle, slot, "Pw", 
				     1, &channel[i], &val);
      if (returnCode) {
         fprintf(stderr, "SetParam ERROR: %s   (%d): ",
                 CAENHV_GetError(SystemHandle), returnCode);
         return 1;
      };
   };
   free(channel);

   return (0);
};


/*****************************************************************************/
//TURN ALL CHANNELS IN SLOT ON                                                /
/*****************************************************************************/
//This function turns all of the channels in a slot on
int Power_All()
{
   int returnCode = 1;
   int slot = 0, temp = 0, i = 0;
   unsigned short *channel = NULL, chNum = 0;
   unsigned val;


   fprintf(stderr, "Please enter the slot number\n");
   scanf("%d", &temp);
   slot = temp;

   chNum = NrOfChList[slot];

   channel = calloc(chNum, sizeof(unsigned short));

   for (i = 0; i < chNum; i++) {
      channel[i] = i;
   };

   val = 1;

   returnCode = SetPar(slot, "Pw", chNum, channel, &val);
   if (returnCode) {
      return 1;
   };

   free(channel);

   return 0;
};


/*****************************************************************************/
//TURN CHANNEL POWER ON OR OFF                                                /
/*****************************************************************************/
//This function turns channel(s) on or off depending on the argument passed into it

int Power(unsigned setting)
{
   int returnCode = 1;
   int slot = 0, temp = 0, i = 0;
   unsigned short *channel = NULL, chNum = 0;
   unsigned val;


   fprintf(stderr, "Please enter the slot number\n");
   scanf("%d", &temp);
   slot = temp;

   if (setting == 1) {
      fprintf(stderr, "Please enter the number of channels to turn on\n");
   };

   if (setting == 0) {
      fprintf(stderr, "Please enter the number of channels to turn off\n");
   };

   scanf("%d", &temp);
   chNum = (unsigned short) temp;

   channel = calloc(chNum, sizeof(unsigned short));

   fprintf(stderr,
           "Please enter the channel numbers, separated by spaces\n");

   for (i = 0; i < chNum; i++) {
      scanf("%d", &temp);
      channel[i] = (unsigned short) temp;
   };

   val = setting;

   for (i = 0; i < chNum; i++) {
      fprintf(stderr, "Channel %d\n", channel[i]);
   };

   returnCode = SetPar(slot, "Pw", chNum, channel, &val);
   if (returnCode) {
      return 1;
   };
   free(channel);

   return 0;
};


/*****************************************************************************/
//SET PARAMETER                                                               /
/*****************************************************************************/
//This function sets a parameter on chosen channels

int SetPar(int slot, char *parameter, unsigned short chNum,
           const unsigned short *channel, void *val)
{
   int returnCode;

   returnCode = CAENHV_SetChParam(SystemHandle, slot, parameter,
				  chNum, channel, val);
   if (returnCode) {
      fprintf(stderr, "SetParam ERROR: %s   (%d): ",
              CAENHV_GetError(SystemHandle), returnCode);
   };
   return returnCode;
};

/*****************************************************************************/
//SET CHANNEL V0                                                              /
/*****************************************************************************/
//This function sets channels' V0

int Voltage()
{
   int returnCode = 1;
   int slot = 0, temp = 0, i = 0;
   unsigned short *channel = NULL, chNum = 0;
   float val, temp2;


   fprintf(stderr, "Please enter the slot number\n");
   scanf("%d", &temp);
   slot = temp;

   fprintf(stderr,
           "Please enter the number of channels to set the voltage of\n");
   scanf("%d", &temp);
   chNum = (unsigned short) temp;

   channel = calloc(chNum, sizeof(unsigned short));

   fprintf(stderr,
           "Please enter the channel numbers, separated by spaces\n");

   for (i = 0; i < chNum; i++) {
      scanf("%d", &temp);
      channel[i] = (unsigned short) temp;
   };

   fprintf(stderr, "Please enter the setting voltage\n");
   scanf("%f", &temp2);
   val = temp2;

   returnCode = SetPar(slot, "V0Set", chNum, channel, &val);
   if (returnCode) {
      return 1;
   };
   free(channel);

   return 0;
};


/*****************************************************************************/
//TEST PROGRAM                                                                /
/*****************************************************************************/
//This function sets the odd-numbered channels' voltages to 1234V, turns them on, and turns them off

int Test_Program()
{
   int slot, returnCode;
   unsigned short channel[4] = { 0, 2, 4, 6 }, chNum = 4;
   float val;
   unsigned val2;

   val = 1234;
   slot = 4;

   returnCode = SetPar(slot, "V0Set", chNum, channel, &val);
   if (returnCode) {
      return 1;
   };

   sleep(5);
   val2 = 1;

   returnCode = SetPar(slot, "Pw", chNum, channel, &val2);
   if (returnCode) {
      return 1;
   };

   sleep(30);
   val2 = 0;

   returnCode = SetPar(slot, "Pw", chNum, channel, &val2);
   if (returnCode) {
      return 1;
   };

   return 0;
};

/*****************************************************************************/
//ECHO SETTING FUNCTION                                                       /
/*****************************************************************************/
//Scott's cool function for making text typed into the console not show up (used for passwords in this program)

void set_echo(int setval)
{
   struct termios settings;
   int result, desc = 0;

   if (!settings_changed) {
      result = tcgetattr(0, &default_settings);
      if (result < 0)
         fprintf(stderr, "Could not get echo settings\n");
   }

   settings = default_settings;

   switch (setval) {
   case 0:                     /* off */
      {
         settings.c_lflag &= ~(ECHO);
         result = tcsetattr(0, TCSANOW, &settings);
         settings_changed = 1;

         if (result < 0)
            fprintf(stderr, "Could not set echo settings\n");
         break;
      }
   case 1:
      {
         result = tcsetattr(0, TCSANOW, &default_settings);
         if (result < 0)
            fprintf(stderr, "Could not set echo settings\n");
         break;
      }
   }
   return;
}
