// command.h: Schnittstelle fuer die Klasse CInterpreter.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMAND_H__CE717DC8_3D70_42BB_AA0A_B9F4C73250A5__INCLUDED_)
#define AFX_COMMAND_H__CE717DC8_3D70_42BB_AA0A_B9F4C73250A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <string.h>
#include <list>
#include "htable.h"
#include "config.h"


class CInterpreter;


#define CMDLINELENGTH 80


bool keypressed();


#define MAXSYMLEN  10

struct CSymbol { char name[MAXSYMLEN+1]; int value; };


class CCmdLine
{
	bool interactive;
	char s[CMDLINELENGTH+1];
	char *cmd, *par;
	bool read(FILE *f);
	bool isCmd(const char name[]) { return strcmp(cmd,name) == 0; }

	static const CSymbol symtable[];

	static bool isAlpha(char ch)
	{ return ('A'<=ch && ch<='Z') || ('a'<=ch && ch<='z'); }
	static bool isNumber(char ch) { return '0'<=ch && ch<='9'; }
	static bool isAlphaNum(char ch) { return isAlpha(ch) || isNumber(ch); }
	static bool isWhitespace(char ch) { return ch==' ' || ch=='\t'; }
	static int GetHex(char ch);
	bool getSymbol(int &value);
	bool getNumber(int &value);
	void setInteractive(bool on) { interactive = on; }
public:
	const char* getName() { return cmd; }
	bool getInt(int &value, int min, int max);
	bool getIntRange(int &valuemin, int &valuemax, int skipmin, int skipmax);
	bool getString(char *value, int size);
	bool getStringEOL(char *value, int size);
	bool isInteractive() { return interactive; }
	friend class CInterpreter;
};


typedef bool(*CMDFUNCTION)(CCmdLine &);

class CCommand
{
	const char *m_name;
	const char *m_parameter;
	const char *m_help;
	CMDFUNCTION m_exec;
public:
	friend class CInterpreter;
};


class CHelpCategory
{
	const char *m_name;
	std::list<CCommand> helpList;
public:
	friend class CInterpreter;
};


class CInterpreter
{
	CHelpCategory *currentHelpCat;
	std::list<CHelpCategory> helpCategory;
	CHashTable<CCommand> cmdList;
	CCmdLine cmdline;
	char scriptPath[256];
	void ListHelpCategories();
	void ListHelpText(std::list<CHelpCategory>::iterator cat);
	void help();
public:
	CInterpreter();
	~CInterpreter() {};
	void SetScriptPath(const char path[]);
	void AddHelpCategory(const char name[]);
	void AddCommand(const char name[], CMDFUNCTION f, const char parameter[], const char help[]);
	bool run(FILE *f, int iter = 0);
};


bool cmd_not_implemented(CCmdLine &par);

extern CInterpreter cmd_intp;

#define HELP_CAT(name)
#define CMD_REG(name,parameter,helptext) bool cmd_##name(CCmdLine &par);
#define CMD_PROC(name) bool cmd_##name(CCmdLine &par)

#define CMD_NUL(name, help) cmd_intp.AddCommand(#name, cmd_not_implemented, help)
#define CMD_RUN(file) cmd_intp.run(file);
#define PAR_INT(var,min,max) if (!par.getInt(var,(min),(max))) \
{ printf("illegal integer parameter!\n"); return false; }
#define PAR_IS_INT(var,min,max) par.getInt(var,(min),(max))

#define PAR_RANGE(varmin,varmax,min,max) if (!par.getIntRange(varmin,varmax,(min),(max))) \
{ printf("illegal range parameter!\n"); return false; }

#define PAR_STRING(var,size) if (!par.getString(var,size)) \
{ printf("illegal string parameter!\n"); return false; }

#define PAR_IS_STRING(var,size) (par.getString(var,size))

#define PAR_STRINGEOL(var,size) if (!par.getStringEOL(var,size)) \
{ printf("illegal string parameter!\n"); return false; }

#endif // !defined(AFX_COMMAND_H__CE717DC8_3D70_42BB_AA0A_B9F4C73250A5__INCLUDED_)
