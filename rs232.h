// rs232.h

#ifndef RS232_H
#define RS232_H

int rs232_open(int portNr, int baud, char parity, int data_bit, int stop_bit,
	int flow_control);
	
int rs232_exit(int fd);

int rs232_write(int fd, char *data, int size);

int rs232_read(int fd, char *str, int size, int timeout);

int rs232_puts(int fd, const char *str);

int rs232_gets(int fd, char *str, int size, const char *pattern, int timeout);

void rs232_clearRx(int fd);

void rs232_clearTx(int fd);

void rs232_clear(int fd);

void rs232_setStatus(int fd, bool rts, bool cts);

#endif
