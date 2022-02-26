#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

int debug_flag=0;

int rs232_open(int portNr, int baud, char parity, int data_bit, int stop_bit,
	int flow_control)
{
int  fd, i;
struct termios tio;

struct
{
  int speed;
  int code;
} baud_table[] =
{
  {300,  B300},
  {600,  B600},
  {1200, B1200},
  {1800, B1800},
  {2400, B2400},
  {4800, B4800},
  {9600, B9600},
  {19200,B19200},
  {38400,B38400},
  {0,0}
};

  char port[20];
  
  if (portNr<0 || 99<portNr) return -1;
  sprintf(port, "/dev/ttyS%i", portNr);
 // sprintf(port, "/dev/cua%i", portNr);
  fd = open(port, O_RDWR);
  if (fd < 0)
    {
    perror("rs232_open");
    return fd;
    }

  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_cflag = CREAD | CLOCAL;
  if (data_bit == 7)
    tio.c_cflag |= CS7;
  else
    tio.c_cflag |= CS8;

  if (stop_bit == 2)
    tio.c_cflag |= CSTOPB;

  if (parity == 'E')
    tio.c_cflag |= PARENB;
  if (parity == 'O')
    tio.c_cflag |= PARENB | PARODD;

  if (flow_control == 1)
    tio.c_cflag |= CRTSCTS;
  if (flow_control == 2)
    tio.c_iflag |= IXON | IXOFF;

  tio.c_lflag = 0;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 0;

  /* check baud argument */
  for (i=0 ; baud_table[i].speed ; i++)
    {
    if (baud == baud_table[i].speed)
      break;
    }

  if (!baud_table[i].speed)
    i = 6; /* 9600 baud by default */

  cfsetispeed(&tio, baud_table[i].code);
  cfsetospeed(&tio, baud_table[i].code);

  tcsetattr(fd, TCSANOW, &tio);

  return fd;
}

/*----------------------------------------------------------------------------*/

int rs232_exit(int fd)
{
  close(fd);

  return 0;
}

/*----------------------------------------------------------------------------*/

int rs232_write(int fd, char *data, int size)
{
int i;

  if (debug_flag)
    {
    FILE *f;

    f = fopen("rs232.log", "a");
    fprintf(f, "write: ");
    for (i=0 ; (int)i<size ; i++)
      fprintf(f, "%X ", data[i]);
    fprintf(f, "\n");
    fclose(f);
    }

  i = write(fd, data, size);

  return i;
}

/*----------------------------------------------------------------------------*/

int rs232_puts(int fd, const char *str)
{
int i;

  if (debug_flag)
    {
    FILE *f;

    f = fopen("rs232.log", "a");
    fprintf(f, "puts: %s\n", str);
    fclose(f);
    }

  i = write(fd, str, strlen(str));

  return i;
}

/*----------------------------------------------------------------------------*/

int rs232_gets(int fd, char *str, int size, const char *pattern, int timeout)
{
	int    i, l;
	struct timeb start, now;
	double fstart, fnow;

	ftime(&start);
	fstart = start.time+start.millitm/1000.0;

	memset(str, 0, size);
	for (l=0 ; l<size-1 ;)
	{
		ioctl(fd, FIONREAD, &i);
		if (i > 0)
		{
			i = read(fd, str+l, 1);
			if (i == 1) l++; else perror("read");
		}

		if (pattern && pattern[0])
			if (strstr(str, pattern) != NULL)
		break;

		ftime(&now);
		fnow = now.time+now.millitm/1000.0;

		if ((fnow - fstart) >= timeout/1000.0)
		{
			if (pattern && pattern[0]) return 0;
			break;
		}

		// if (i == 0)
			// usleep(min(100000, timeout*1000));
			// usleep(timeout*1000);
	};

	return l;
}

/*----------------------------------------------------------------------------*/

void rs232_clearRx(int fd)
{
	tcflush(fd, TCIFLUSH);
}


void rs232_clearTx(int fd)
{
	tcflush(fd, TCOFLUSH);
}


void rs232_clear(int fd)
{
	tcflush(fd, TCIOFLUSH);
}


/*----------------------------------------------------------------------------*/

void rs232_setStatus(int fd, bool rts, bool cts)
{
	int status;
	ioctl(fd, TIOCMGET, &status);
	if (rts) status |= TIOCM_RTS; else status &= ~TIOCM_RTS;
	if (cts) status |= TIOCM_CTS; else status &= ~TIOCM_CTS;
	ioctl(fd, TIOCMSET, &status);
}
