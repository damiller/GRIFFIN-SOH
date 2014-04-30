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

int works = 0;

//VARIABLES FOR CHANGING

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

//File name starts for each channel (use channel numbers from the Channel array)
char FName[4][20] = {"ch101_",
		     "ch102_",
		     "ch103_",
		     "ch104_"};

//FUNCTIONS
int rs232_init(int port, int baud, char parity, int data_bit, int stop_bit, int mode);
void close_rs232(int hDev);
int Init();
void Clr_Buf();
int Send_Cmd(int hDev, char *str);
int Get_Resp(int hDev);
void Write_File(char *fDes, char *str, int ccat);
void Set_Time(int hDev);

main()
{
  int rCode = 0;
  rCode = Init();
  return;
};
 

int Init()
{
  int rCode = 0, hDev = 0, i = 0, j = 2, year, month, day, hour, minute, second;
  char buffer[100], *str, *tmstr, cmd[256], fName[48];
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


  /*Set up scan reading formatting (the information to display in each reading) 
  Each of the pieces of information below can be ON or OFF except TIME:TYPE, which can be the actual 
  time and date (ABS) or the time since the start of the scan (REL)*/
  rCode = Send_Cmd(hDev, "ROUT:SCAN (@101)\n");
  if (rCode <= 0)
    {
      return 1;
    };
  
    /* rCode = Send_Cmd(hDev, "ROUT:SCAN?\n");  
       sleep (1);
       rCode = Get_Resp(hDev);  
       fprintf(stdout, "%s\n", Buffer);*/

  rCode = Send_Cmd(hDev, "FORM:READ:ALAR OFF\n");  //Display alarm data?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:UNIT ON\n");  //Display measurement units?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:CHAN ON\n");  //Display channel number?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:TIME ON\n");  //Display time data?
  if (rCode <= 0)
    {
      return 1;
    };
  rCode = Send_Cmd(hDev, "FORM:READ:TIME:TYPE ABS\n");  //ABS(olute) or REL(ative) time?
  if (rCode <= 0)
    {
      return 1;
    };

  //Get time to add to add to channel number to create filename
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
	  rCode = sprintf (cmd, "SENS:TEMP:TRAN:TYPE RTD,(@%d)\n", Channel[i]); 
	  rCode = Send_Cmd(hDev, cmd);
	  rCode = sprintf (cmd, "SENS:TEMP:TRAN:RTD:TYPE 85,(@%d)\n", Channel[i]); 
	  rCode = Send_Cmd(hDev, cmd);
	  rCode = sprintf (cmd, "SENS:TEMP:NPLC 1,(@%d)\n", Channel[i]); 
	  rCode = Send_Cmd(hDev, cmd);
	  rCode = sprintf (cmd, "UNIT:TEMP K,(@%d)\n", Channel[i]);
	  rCode = Send_Cmd(hDev, cmd);
	}
      else
	{
	  if (Range[i] == "AUTO")
	    {
	      rCode = sprintf (cmd, "SENS:%s:RANG:AUTO ON,(@%d)\n", Type[i], Channel[i]);
	      rCode = Send_Cmd(hDev, cmd);
	    }
	  else
	    {
	      rCode = sprintf (cmd, "SENS:%s:RANG %s,(@%d)\n", Type[i], Range[i], Channel[i]);
	      rCode = Send_Cmd(hDev, cmd);
	    };

	  Clr_Buf(cmd);

	  rCode = sprintf (cmd, "SENS:%s:RES %s,(@%d)\n", Type[i], Resolution[i], Channel[i]);     
	  rCode = Send_Cmd(hDev, cmd); 	      
	};
      rCode = sprintf (cmd, "CONF? (@%d)\n", Channel[i]);
      rCode = Send_Cmd(hDev, cmd);
      usleep (500000);
      rCode = Get_Resp(hDev);
      sprintf (buffer, "Channel %d Configuration: %s", Channel[i], Buffer);
      Write_File(fName, buffer, 0);
      Clr_Buf(Buffer);
    };

  //Test scan
  while(1)
    {
      rCode = Send_Cmd(hDev, "TRIG:SOUR IMM\n");  //setup trigger mode
      rCode = Send_Cmd(hDev, "TRIG:COUN 1\n");  //setup number of sweeps
      rCode = Send_Cmd(hDev, "READ?\n");  //start scan
      time(&tm);
      tmstr = asctime(localtime(&tm));
      tmstr[24] = 0;
      usleep (500000);
      rCode = Get_Resp(hDev);  //Get reading from output buffer
      sprintf (buffer, "%s       %s", tmstr, Buffer);
      Write_File(fName, buffer,1); 
      fprintf(stdout, "%s\n", buffer);
      fflush (stdout);
      
      for(i = 0; i < 63; i++)
	{
	  usleep (500000);
	};
      //   j--;
    };

  //Get date and time
  /*  time(&tm);
    tmstr = asctime(localtime(&tm));
  fprintf (stdout, "%s\n", tmstr);*/

  //Close the Serial Port
  close_rs232(hDev);
  
  return 0;
};

void Clr_Buf(char* buf)
{
  int j = 0;
  for (j = 0; j < strlen(buf); j++)
    {buf[j] = '\0';};
}


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

int Get_Resp(int hDev)
{
  int rCode; //stores number of bytes read  

  fcntl(hDev, F_SETFL, FNDELAY); //sets port for no wait
  rCode = read( hDev, Buffer, 256);
  if (rCode <= 0)
    {
      fprintf (stderr, "\nError: No Data Read! (%d)\n", rCode);
    };
  return rCode;
};

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

void close_rs232(int hDev)
 {
   close(hDev);
   return;
 };
