#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>


//Global Variable Definitions
char *Buffer = NULL;

//*****************************************************************************
//VARIABLES TO CHANGE
//*****************************************************************************/

//Desired time between each scan (in seconds)
int Intval = 3;

//Desired number of channels in the scan list 
int NumCh = 3;

//Desired channels in the scan list
int Channel[3] = {101, 102, 103};

//Desired function for each channel in above list (in order)
char *Type[3] = {"TEMP",
		 "VOLT:DC",
		 "RES"};

//Desired range for each channel in the above list (in order)
char *Range[3] = {"DEF",
		  "AUTO",
		  "AUTO"};

char *Resolution[3] = {"MAX",
		       "MAX",
		       "MAX"};
		       






/*

int Intval = 2;


//Desired number of channels in the scan list 
int NumCh = 4;

//Desired channels in the scan list
int Channel[4] = {101, 102, 103, 104};

//Desired function for each channel in above list (in order)
char *Type[4] = {"TEMP",
		 "RES",
		 "TEMP",
		 "TEMP"};

char *Range[4] = {"DEF",
		  "AUTO",
		  "DEF",
		  "DEF"};

char *Resolution[4] = {"MAX",
		       "MAX",
		       "MAX",
		       "MAX"};










//end of variables for changing
//*****************************************************************************/


//FUNCTIONS
int rs232_init(int port, int baud, char parity, int data_bit, int stop_bit, int mode);
void close_rs232(int hDev);
int Init();
void Clr_Buf();
int Send_Cmd(int hDev, char *str);
int Get_Resp(int hDev);
void Write_File(char *fDes, char *str, int ccat);
void Set_Time(int hDev);

//***************************************************************************//
// MAIN FUNCTION                                                             //
//***************************************************************************//
main()
{
  int rCode = 0;
  rCode = Init();
  return;
};
 
//***************************************************************************//
// CHANNEL READOUT FUNCTION                                                  //
//***************************************************************************//
int Init()
{
  int rCode = 1, hDev = 0, i = 0, j = 2, year, month, day, hour, minute, second;
  char buffer[256], *str, *tmstr, cmd[256], fName[48], channel[3];
  time_t tm;
  struct tm *tPtr, tme;
  
  Buffer = malloc (256*sizeof(char));
  str = malloc (256*sizeof(char));

  //Open and Initialize the Serial Port
  hDev = rs232_init(0, 9600, 'E', 7, 1, 0);

  //Set_Time(hDev);

 //Test connection by querying slot 100 and printing to screen
  str = "SYST:CTYPE? 100\n";
  rCode = Send_Cmd(hDev, str);
  if (rCode <= 0)
    {
      return 1;
    };
  usleep (100000);
  rCode = Get_Resp(hDev);
  if (rCode <= 0)
    {
      return 1;
    };
  fprintf (stderr, "Module in Slot 100: %s\n", Buffer);

  //Resets instrument and clear status byte
  str = "*CLS\n";
  rCode = Send_Cmd(hDev, str);

  //Setup scan list
  Clr_Buf(buffer);
  sprintf(buffer, "ROUT:SCAN (@%d", Channel[0]);
  for(i = 1; i < NumCh; i++)
    {
      sprintf(channel, ",%d", Channel[i]);
      strcat(buffer, channel);
    };
  strcat(buffer, ")\n");
//  fprintf (stdout, buffer);

  rCode = Send_Cmd(hDev, buffer);
  if (rCode <= 0)
    {
      return 1;
    };

  Clr_Buf(buffer);

  /*Set up scan reading formatting (the information to display in each reading) 
  Each of the pieces of information below can be ON or OFF except TIME:TYPE, which can be the actual 
  time and date (ABS) or the time since the start of the scan (REL)*/

  rCode = Send_Cmd(hDev, "FORM:READ:ALAR OFF\n");  //Display alarm data?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:UNIT OFF\n");  //Display measurement units?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:CHAN OFF\n");  //Display channel number?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:TIME OFF\n");  //Display time data?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:TIME:TYPE ABS\n");  //ABS(olute) or REL(ative) time?
  if (rCode <= 0)
    {
      return 1;
    };

  //Get time and create filename
  time(&tm);
  tPtr = malloc(sizeof(struct tm));
  tPtr = localtime(&tm);
  tme = *tPtr;
  year = (1900 + tme.tm_year);
  month = (tme.tm_mon + 1);
  day = tme.tm_mday;
  hour = tme.tm_hour;
  minute = tme.tm_min;
  second = (float) tme.tm_sec;
  sprintf (fName, "%d-%d-%d_%d:%d:%d", year, month, day, hour, minute, second);

  //Configure channels
  for (i = 0; i < NumCh; i++)
    {
      //Initialize channel
      rCode = sprintf (cmd, "SENS:FUNC '%s',(@%d)\n", Type[i], Channel[i]);
      rCode = Send_Cmd(hDev, cmd);
      if (Type[i] == "TEMP")
	{
	  //If the type is temperature, different parameters must be specified.          //It is assumed in this code that the temperature sensor will be an 
	  //85-type RTD with an integration time of 1 and units in K, but all
	  //of these can be adjusted in the following lines..
	  rCode = sprintf (cmd, "SENS:TEMP:TRAN:TYPE RTD,(@%d)\n", Channel[i]); 
	  rCode = Send_Cmd(hDev, cmd);
	  rCode = sprintf (cmd, "SENS:TEMP:TRAN:RTD:TYPE 85,(@%d)\n", Channel[i]); 
	  rCode = Send_Cmd(hDev, cmd);
	  rCode = sprintf (cmd, "SENS:TEMP:NPLC 1,(@%d)\n", Channel[i]); 
	  rCode = Send_Cmd(hDev, cmd);
	  rCode = sprintf (cmd, "UNIT:TEMP K,(@%d)\n", Channel[i]);
	  rCode = Send_Cmd(hDev, cmd);
	}
      else //if configuration is anything but temperature
	{
	  if (Range[i] == "AUTO") //turn autoranging on
	    {
	      rCode = sprintf (cmd, "SENS:%s:RANG:AUTO ON,(@%d)\n", Type[i], Channel[i]);
	      rCode = Send_Cmd(hDev, cmd);
	    }
	  else //set range to that specified as global variable
	    {
	      rCode = sprintf (cmd, "SENS:%s:RANG %s,(@%d)\n", Type[i], Range[i], Channel[i]);
	      rCode = Send_Cmd(hDev, cmd);
	    };

	  Clr_Buf(cmd);

	  rCode = sprintf (cmd, "SENS:%s:RES %s,(@%d)\n", Type[i], Resolution[i], Channel[i]);     
	  rCode = Send_Cmd(hDev, cmd); 	      
	};
      // Get and print out channel configuration for each channel at top of file
      rCode = sprintf (cmd, "CONF? (@%d)\n", Channel[i]);
      rCode = Send_Cmd(hDev, cmd);
      usleep (500000);
      rCode = Get_Resp(hDev);
      sprintf (buffer, "Channel %d Configuration: %s", Channel[i], Buffer);
      if (i == 0)     {Write_File(fName, buffer, 0);}
      else           {Write_File(fName, buffer, 1);};
      Clr_Buf(Buffer);
    };  //end for

      rCode = Send_Cmd(hDev, "TRIG:SOUR IMM\n");  //setup trigger mode
      rCode = Send_Cmd(hDev, "TRIG:COUN 1\n");  //setup number of sweeps

  //Test scan
  while(1)
    {

      rCode = Send_Cmd(hDev, "READ?\n");  //start scan
      //Create timestamp
      time(&tm);
      tmstr = asctime(localtime(&tm));
      tmstr[24] = 0;
      //sleep to wait for response
      usleep (500000*NumCh);
      //Get reading from output buffer and print to screen and file
      rCode = Get_Resp(hDev);  
      sprintf (buffer, "%s       %s", tmstr, Buffer);
      Write_File(fName, buffer,1); 
      fprintf(stdout, "%s\n", buffer);
      fflush (stdout);

      //Clear buffers (to prevent overwrite)
      Clr_Buf(buffer);
      Clr_Buf(Buffer);

      //Sleep for time interval given as global variable
      for(i = 0; i < (2*Intval - NumCh); i++)
	{
	  usleep (500000);
	};
    };  //end while

  //Close the Serial Port
  close_rs232(hDev);
  
  //deallocate memory
  return 0;
};


//***************************************************************************//
// BUFFER CLEAR FUNCTION                                                     //
//***************************************************************************//
void Clr_Buf(char* buf)
{
  int j = 0;
  for (j = 0; j < 256; j++)
    {buf[j] = '\0';};
}


//***************************************************************************//
// WRITE DATA TO FILE FUNCTION                                               //
//***************************************************************************//
void Write_File(char *fDes, char *str, int ccat)
{
  FILE *fPo;
  if(ccat == 0)
    {
      fPo = fopen(fDes, "w");
      if (fPo == NULL)
	{
	  fprintf (stderr, "ERROR: File '%s' could not be opened\n", fDes);
	};
    };
  
  if(ccat == 1)
    {
      fPo = fopen(fDes, "a");
      if (fPo == NULL)
	{
	  fprintf (stderr, "ERROR: File '%s' could not be opened\n", fDes);
	};
    };

  fprintf (fPo, "%s\n", str);
  fflush (fPo);
  
  fclose(fPo);

  return;
}


//***************************************************************************//
// WRITE DATA TO SERIAL PORT FUNCTION                                        //
//***************************************************************************//
int Send_Cmd(int hDev, char *str)
{
  int rCode; //stores number of bytes sent

  rCode = write(hDev, str, strlen(str));
  if (rCode <= 0)
    {
      fprintf (stderr, "ERROR: Write of %s (%d bytes) failed\n", str, strlen(str));
    };  

  return rCode;
};


//***************************************************************************//
// READ DATA FROM SERIAL PORT FUNCTION                                       //
//***************************************************************************//
int Get_Resp(int hDev)
{
  int rCode; //stores number of bytes read  

  fcntl(hDev, F_SETFL); //sets port for no wait , FNDELAY
  rCode = read( hDev, Buffer, 256);
  if (rCode <= 0)
    {
      fprintf (stderr, "\nError: No Data Read! (%d)\n", rCode);
    };
  return rCode;
};


//***************************************************************************//
// SET DMM TIME/DATE FUNCTION                                                //
//***************************************************************************//
void Set_Time(int hDev)
{
  char cmd[256], *tmstr;
  int rCode, year, month, day, hour, minute;
  float second;
  time_t tmer;
  struct tm *tPtr, tme;

  tPtr = malloc(sizeof(struct tm));

  time(&tmer);
  tPtr = localtime(&tmer);
  tme = *tPtr;

  year = (1900 + tme.tm_year);
  month = (tme.tm_mon + 1);
  day = tme.tm_mday;
  hour = tme.tm_hour;
  minute = tme.tm_min;
  second = (float) tme.tm_sec;

  rCode = sprintf (cmd, "SYST:DATE %4d,%2d,%d\n", year, month, day); 
  fprintf (stdout, "%s\n", cmd);
  rCode = Send_Cmd(hDev, cmd);
  rCode = sprintf (cmd, "SYST:TIME %2d,%2d,%5.3f\n", hour, minute, second); 
  fprintf (stdout, "%s\n", cmd);
  rCode = Send_Cmd(hDev, cmd);


  tmstr = asctime(localtime(&tmer));
  fprintf (stdout, "Time Set to %s\n", tmstr);
};


//*****************************************************************************
//INITIALIZE PORT FUNCTION FROM RS232.C 
int rs232_init(int port, int baud, char parity, int data_bit, int stop_bit, int mode)
{
  {
  char str[80];
  int hDev, i=-1, status;
  struct termios termios_p;
  speed_t speed;
  
  struct {
      int speed;
      int code;
  } Baud[10] = {
    {300, 7}, {600,10}, {1200,11},
    {1800,12}, {2400,13}, {4800,14}, {9600,15},
    {19200,16}, {38400,17}, {0,0}
  };

  /* check baud argument */
  while (Baud[++i].speed)
  {
    if (baud == Baud[i].speed)
      break;
  }
  if (Baud[i].speed == 0)
  {
    fprintf(stderr,"rs232_init: Unvalid baud speed (%d)",baud);
    return 0;
  }
  baud = Baud[i].code;
  
  sprintf(str, "/dev/ttyS%d", port);


  hDev = open (str, O_RDWR | O_NONBLOCK | O_NDELAY);
  if (hDev < 0)
  {
    fprintf(stderr,"rs232_init: Can't open serial port %s",str);
    hDev = 0;
    return hDev;
  }

  /* Port setup */
  status =  tcgetattr( (int) hDev, &termios_p);
  /* input */
  termios_p.c_iflag &= ~(ICRNL|IXON);
  termios_p.c_iflag |= IGNBRK;

  /* output */
  termios_p.c_oflag &= ~(OPOST|ONLCR);
  
  /* control */
  termios_p.c_cflag &= ~(CRTSCTS | CSIZE);
  termios_p.c_cflag |= (CS8);
  
  /* l flags */
  termios_p.c_lflag &= ~(ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK|ECHOKE|ECHOCTL);
  
  status =  tcsetattr( (int) hDev, TCSANOW, &termios_p);

/*
  speed =  cfgetispeed (&termios_p);
  printf("getispeed status =%d, ospeed:%d \n", status, speed);
  speed =  cfgetospeed (&termios_p);
  printf("getospeed status =%d, ispeed:%d \n", status, speed);
*/
  /* set BAUD speed */
//  status =  cfsetispeed(&termios_p, (speed_t) baud);
//  printf(" setispeed status =%d errno %s\n", status, strerror(errno));
//  status =  cfsetospeed(&termios_p, (speed_t) baud);
//  printf(" setospeed status =%d errno %s\n", status, strerror(errno));
 
//  status =  tcsetattr( (int) hDev, TCSANOW, &termios_p);
//  printf(" tcsetattr status =%d errno %s\n", status, strerror(errno));
/*
  speed =  cfgetispeed (&termios_p);
  printf("getispeed status =%d, ospeed:%d \n", status, speed);
  speed =  cfgetospeed (&termios_p);
  printf("getospeed status =%d, ispeed:%d \n", status, speed);
*/
  return (int) hDev;
  };
};


//***************************************************************************//
// CLOSE SERIAL PORT FUNCTION                                                //
//***************************************************************************//
void close_rs232(int hDev)
 {
   close(hDev);
   return;
 };
