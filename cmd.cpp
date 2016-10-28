/* -------------------------------------------------------------
 *
 *  file:        command.cpp
 *
 *  description: command line interpreter for Chip/Wafer tester
 *
 *  author:      Beat Meier
 *  modified:    31.8.2007
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#include "psi46test.h"
#include "command.h"

#include "cmd_dtb.h"
#include "cmd_wafertest.h"
#include "cmd_analyzer.h"
#include "cmd_stb.h"


void cmdHelp()
{
	if (settings.proberPort >= 0)
	{
	 fputs("\n"
	 "+-- control commands ------------------------------------------+\n"
	 "| h                  display this text                         |\n"
	 "| exit               exit commander                            |\n"
	 "+-- wafer test ------------------------------------------------+\n"
	 "| go                 start wafer test (press <cr> to stop)     |\n"
	 "| test               run chip test                             |\n"
	 "| pr <command>       send command to prober                    |\n"
	 "| sep                prober z-axis separation                  |\n"
	 "| contact            prober z-axis contact                     |\n"
	 "| first              go to first die and clear wafer map       |\n"
	 "| next               go to next die                            |\n"
	 "| goto <x> <y>       go to specifed die                        |\n"
	 "| chippos <ABCD>     move to chip A, B, C or D                 |\n"
	 "+--------------------------------------------------------------+\n",
	 stdout);
	}
	else
	{
	 fputs("\n"
	 "+-- control commands ------------------------------------------+\n"
	 "| h                  display this text                         |\n"
	 "| exit               exit commander                            |\n"
	 "+-- chip test -------------------------------------------------+\n"
	 "| test <chip id>     run chip test                             |\n"
	 "+--------------------------------------------------------------+\n",
	 stdout);
	}
}


CMD_PROC(h)
{
	cmdHelp();
}



void cmd()
{
#undef  CMD_REG
#define CMD_REG(name,parameter,helptext) cmd_intp.AddCommand(#name, cmd_##name, parameter, helptext);
#undef HELP_CAT
#define HELP_CAT(name) cmd_intp.AddHelpCategory(name);

#include "cmd_dtb.h"
#include "cmd_wafertest.h"
#include "cmd_analyzer.h"
#include "cmd_stb.h"


	CMD_REG(h, "", "simple help");

	cmdHelp();

	cmd_intp.SetScriptPath(settings.scriptPath.c_str());

	// command loop
	while (true)
	{
		try
		{
			CMD_RUN(stdin);
			return;
		}
		catch (CRpcError e)
		{
			e.What();
		}
	}
}

