#include <stdio.h>
#include <string.h>

int main (void)
{
  char tmp[80]={'\0'}, str[80]={'\0'}, cmd[80]={"*CLS"};
  char resp[512];
  int  hDev = 0, nbytes=0, ntotbytes = 0, nreads = 0, status;
  int iValue, crate, slot;
  int ii,len;
  char sstr[16], sign[2];
  float  value = 0.034;
  int    channel=43;
  char* qm;
/*
  SOURCE:VOLT 5.777,(@304)
  SOURCE:VOLT 4.555,(@304)
*/
  hDev = rs232_init(1, 57600, 'N', 8, 1, 0);
  
  if (hDev)
  {
    while (1)
    {
      printf("mtest> [%s] :",cmd);
      ss_gets(str,128);
      if (str[0] == 'q')
      {
	printf("\n");
	break;
      }
      if (strlen(str) == 0)
	strcpy(tmp, cmd);
      else
	strcpy(tmp, str);

      strcpy (cmd, tmp);
      sprintf(tmp, "%s\n",cmd);
      printf("sending:%s\n", tmp);
      rs232_puts(hDev, tmp);

      if (strchr(tmp, '?'))
      {
	usleep(500000);
	nreads = 0; ntotbytes = 0;
	do {
	  usleep(500000);
	  // nbytes = rs232_gets_wait(hDev, resp, 512, '\n');
	  nbytes = rs232_gets_nowait(hDev, resp+ntotbytes, 512, '\n');
	  ntotbytes += nbytes;
	  nreads++;
	  if (nreads > 10) {
	    printf("\n Too many reads. Aborting!\n");
	    break;
	  }
	} while (nbytes > 0);
	printf("\nreceiving: nbytes:%d in nreads:%d\n|%s|\n",ntotbytes, nreads, resp);
      }
    }
  }
  rs232_exit(hDev); 
}
