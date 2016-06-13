/* -------------------------------------------------------------
 *
 *  file:        command.h
 *
 *  description: connection to the Suess prober via RS232
 *
 *  author:      Beat Meier
 *  modified:    13.6.2016
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
	int result;
	const char *message;

	bool IsOpen() { return rs232 >= 0; }
	void Clear();
	void Read(int ms = 6000);

public:
	CProber();
	bool Open (int portNr);
	void Close ();
	~CProber() { Close(); }
	int SendCmd(const char *fmt, ...);
	int GetRsp() { return result; }
	const char* GetRspString() { return readback; }
	const char* GetParamString() { return message; }
	void PrintErrorMsg();
};


#endif
