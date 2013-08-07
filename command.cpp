// command.cpp

#include "command.h"

#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sys/ioctl.h>
#endif


CInterpreter cmd_intp;


#ifdef _WIN32

bool keypressed()
{
	if (!_kbhit()) return false;
	do _getch(); while (_kbhit());
	return true;
}

#else

bool _kbhit()
{
	int n;
	ioctl(0, FIONREAD, &n);
	return n > 0;
}

bool keypressed()
{
	if (!_kbhit()) return false;
	do getchar(); while (_kbhit());
	return true;
}

#endif


// === CCmdLine ==========================================================

bool CCmdLine::read(FILE *f)
{
#ifdef _WIN32
	if (isInteractive()) fputc('>', stdout);
	if (fgets(s, CMDLINELENGTH, f) == NULL) return false;
#else
	if (isInteractive())
	{
		fputc('>', stdout);
		if (fgets(s, CMDLINELENGTH, f) == NULL) return false;
	}
	else
	{
		char * line = readline("> ");
		strncpy(s, line, CMDLINELENGTH);
		add_history(s);
		free(line);
	}
#endif

	int i = 0;
	while (s[i] != 0 && i < CMDLINELENGTH)
	{
		if (s[i] == '\n' || s[i] == '\r') {	s[i] = 0; break; }
		i++;
	}

	cmd = s;
	while (isWhitespace(cmd[0])) cmd++;

	par = cmd;
	while (isAlphaNum(*par)) par++;
	if (par[0] != 0) { par[0] = 0; par++; }

	return true;
}


int CCmdLine::GetHex(char ch)
{
	if (isNumber(ch)) return ch - '0';
	if ('A'<=ch && ch<='F') return ch - 'A' + 10;
	if ('a'<=ch && ch<='f') return ch - 'a' + 10;
	return -1;
}



const CSymbol CCmdLine::symtable[] =
{
	{ "vdig",		0x01 },
	{ "vana",		0x02 },
	{ "vsh",		0x03 },
	{ "vcomp",		0x04 },
	{ "leak",		0x05 },
	{ "rgpr",		0x06 },
	{ "wllpr",		0x07 },
	{ "rgsh",		0x08 },
	{ "wllsh",		0x09 },
	{ "hlddel",		0x0A },
	{ "trim",		0x0B },
	{ "thr",		0x0C },
	{ "bias_bus",	0x0D },
	{ "bias_sf",	0x0E },
	{ "offset_op",	0x0F },
	{ "bias_op",	0x10 },
	{ "offset_ro",	0x11 },
	{ "ion",		0x12 },
	{ "bias_ph",	0x13 },
	{ "bias_dac",	0x14 },
	{ "bias_roc",	0x15 },
	{ "color",		0x16 },
	{ "npix",		0x17 },
	{ "sumcol",		0x18 },
	{ "cal",		0x19 },
	{ "caldel",		0x1A },
	{ "rangetemp",	0x1B },
	{ "wbc",		0xFE },
	{ "ctrl",		0xFD },
	{ "",			0    }
};


bool CCmdLine::getSymbol(int &value)
{
	int n = 0;
	char s[MAXSYMLEN+1];

	while (n<MAXSYMLEN && (isAlphaNum(*par) || (*par == '_')))
	{
		s[n++] = par[0];
		par++;
	}
	s[n] = 0;

	if (n == 0) return false;

	int i = 0;
	while (symtable[i].name[0])
	{
		if (strcmp(s,symtable[i].name) == 0)
		{
			value = symtable[i].value;
			return true;
		}
		i++;
	}

	return true;
}


bool CCmdLine::getNumber(int &value)
{
	int i, x;
	int base;
	int cnt;

	bool neg = false;
	if (par[0] == '-') { neg = true; par++; }
	else if (par[0] == '+') par++;

	switch (par[0])
	{
		case 'b':
		case 'B': base =  2; cnt = 31; par++; break;
		case '$': base = 16; cnt =  7; par++; break;
		default:  base = 10; cnt =  9;
	}

	value = GetHex(par[0]);
	if (value < 0 || base <= value) return false;
	par++;

	for (i=0; i<cnt; i++)
	{
		x = GetHex(par[0]);
		if (0 <= x && x < base) value = value*base + x;
		else break;
		par++;
	}

	if (neg) value = -value;

	x = GetHex(par[0]);
	if (0 <= x && x < base) return false;

	return true;
}


bool CCmdLine::getInt(int &value, int min, int max)
{
	int v;

	while (isWhitespace(par[0])) par++;

	if (par[0] == '#') { par++; if (!getSymbol(v)) return false;	}
	else if (!getNumber(v)) return false;

	if (v < min || max < v) return false;

	value = v;
	return true;
}


bool CCmdLine::getIntRange(int &valuemin, int &valuemax, int skipmin, int skipmax)
{  // nn:nn

	int vmin, vmax;

	// scan first number
	while (isWhitespace(par[0])) par++;
	if (par[0] != ':')
	{
		if (!getInt(vmin, -10000, 10000)) return false;
	}
	else vmin = skipmin;

	// scan for ":"
	if (par[0] == ':')
	{ // scan second number
		par++;
		if (isWhitespace(par[0]) || (par[0] == 0)) vmax = skipmax;
		else
		{
			if (!getInt(vmax, -10000, 10000)) return false;
		}

	}
	else vmax = vmin;

	// check range
	if (vmin>vmax) return false;

	if (vmax<skipmin || vmin>skipmax) return false;

	valuemin = (vmin<skipmin)? skipmin : vmin;
	valuemax = (vmax>skipmax)? skipmax : vmax;
	return true;
}


bool CCmdLine::getString(char *value, int size)
{
	while (isWhitespace(par[0])) par++;

	int i=0;
	while ((i < (size-1)) && !isWhitespace(par[0]) && (par[0] != 0))
	{
		value[i] = par[0];
		par++;
		i++;
	}
	value[i] = 0;
	return i > 0;
}


bool CCmdLine::getStringEOL(char *value, int size)
{
	while (isWhitespace(par[0])) par++;

	int i=0;
	while ((i < (size-1)) && (par[0] != 0))
	{
		value[i] = par[0];
		par++;
		i++;
	}
	value[i] = 0;
	return i > 0;
}



// === CInterpreter ======================================================

CInterpreter::CInterpreter()
{
	memset(scriptPath, 0, 256);
}


void CInterpreter::help()
{
	printf(" help         display this message\n");
	printf(" exit         exit commander\n");
	
	CCommand* p = cmdList.GetFirst();
	while(p)
	{
		printf(" %s\n", p->m_help);
		p = cmdList.GetNext();
	}
}


void CInterpreter::SetScriptPath(const char path[])
{
	strncpy(scriptPath, path, 254);
}


void CInterpreter::AddCommand(const char name[], CMDFUNCTION f, const char help[])
{
	CCommand c;
	c.m_help = help;
	c.m_exec = f;
	cmdList.Add(name, c);
}


bool CInterpreter::run(FILE *f, int iter)
{
	if (iter > 20) return false;

	bool interactive = f==stdin;

	cmdline.setInteractive(interactive);
	while (true)
	{
		if (!cmdline.read(f)) break;
		if (cmdline.getName()[0] == 0) continue;   // empty line
		if (cmdline.getName()[0] == '-') continue; // comment
		CCommand *p = cmdList.Find(cmdline.getName());
		if (p) { p->m_exec(cmdline); }
		else if (cmdline.isCmd("help")) help();
		else if (cmdline.isCmd("exit")) break;
		else
		{
			char *fname = new char[CMDLINELENGTH+256+8];
			strcpy(fname,scriptPath);
#ifdef _WIN32
			strcat(fname, "\\");
#else
			strcat(fname, "/");
#endif
			strcat(fname,cmdline.getName());
			strcat(fname,".roc");
			FILE *cf = fopen(fname, "rt");
			delete[] fname;
			if (cf)
			{
				run(cf, iter+1);
				fclose(cf);
				cmdline.setInteractive(interactive);
			}
			else printf("unknown command \"%s\"!\n", cmdline.getName());
		}
	}
	return true;
}


bool cmd_not_implemented(CCmdLine &par)
{
	printf("command not implemented!\n");
	return true;
}
