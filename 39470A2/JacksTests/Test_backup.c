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


//Channels
int NumCh = 4;

int Channel[4] = {101, 102, 103, 104};

char *Type[4] = {"TEMP RTD,DEF,1,MAX",
		 "VOLT:DC AUTO,MAX",
		 "RES AUTO,MAX",
		 "TEMP RTD,DEF,1,MAX"};

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
void Write_File(char *fDes, char *str);

main()
{
  int rCode = 0;

  rCode = Init();
  return;
};
 

int Init()
{
  int rCode = 0, hDev = 0, i = 0;
  char buffer[100], *str, *tmstr, cmd[256];
  time_t tm;
  
  Buffer = malloc (256*sizeof(char));
  str = malloc (256*sizeof(char));

  //Open and Initialize the Serial Port
  hDev = rs232_init(0, 9600, 'E', 7, 1, 0);

 //Test connection by querying slot 100 and printing to screen
  str = "SYST:CTYPE? 100\n";
  rCode = Send_Cmd(hDev, str);
  sleep (1);
  rCode = Get_Resp(hDev);
  fprintf (stdout, "Module in Slot 100: %s", Buffer);

  //Resets instrument and clear status byte
  str = "*CLS\n";
  rCode = Send_Cmd(hDev, str);

  //Configure channels
  for (i = 0; i < NumCh; i++)
    {
      //Get time to add to add to channel number to create filename
      time(&tm);
      rCode = sprintf (cmd, "%d", tm);
      strcat(FName[i], cmd);      
      fprintf(stdout, "%s\n\n", FName[i]);
      //Initialize channel
      rCode = sprintf (cmd, "CONF:%s,(@%d)\n", Type[i], Channel[i]);
      rCode = Send_Cmd(hDev, cmd);
      rCode = sprintf (cmd, "CONF? (@%d)\n", Channel[i]);
      rCode = Send_Cmd(hDev, cmd);
      Clr_Buf();
      sleep (1);
      rCode = Get_Resp(hDev);
      sprintf (buffer, "Channel %d Configuration: %s", Channel[i], Buffer);
      Write_File(FName[i], buffer);
    };

  //Get date and time
  time(&tm);
  tmstr = asctime(localtime(&tm));
  fprintf (stdout, "%s\n", tmstr);

  //Close the Serial Port
  close_rs232(hDev);
  
  return 0;
};

void Clr_Buf()
{
  int j = 0;
  for (j = 0; j < 256; j++)
    {Buffer[j] = '\0';};
}


void Write_File(char *fDes, char *str)
{
  FILE *fPo;

  fPo = fopen(fDes, "w");
  if (fPo == NULL)
    {
      fprintf (stderr, "ERROR: File '%s' could not be opened\n", fDes);
    };

  fprintf (fPo, "%s\n", str);
  
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
