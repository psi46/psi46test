/* -------------------------------------------------------------
 *
 *  file:        command.h
 *
 *  description: connection to the Suess prober via RS232
 *
 *  author:      Beat Meier
 *  modified:    24.1.2004
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#ifndef PROBER_H
#define PROBER_H


class CProber
{
	int rs232;
	char readback[256];

	bool isOpen() { return rs232 >= 0; }
	void clear();
	char* read(int ms = 6000);

public:
	CProber() { rs232 = -1; readback[0] = 0; }
	bool open (int portNr);
	void close ();
	~CProber() { close(); }
	char* printf(const char *fmt, ...);
	char* getLastResponse() { return readback; }
};


#endif
