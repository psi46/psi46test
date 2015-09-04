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


void cmdHelp()
{
	if (settings.proberPort >= 0)
	{
	 fputs("\n"
	 "+-- control commands ------------------------------------------+\n"
	 "| h                  display this text                         |\n"
	 "| exit               exit commander                            |\n"
	 "+-- wafer test ------------------------------------------------+\n"
	 "| initdiced          initialize chip wafer test after dicing   |\n"
	 "| testdiced          run chip test after dicing                |\n"
 	 "| checkCont          run script to check probes contact        |\n"
  	 "| checkphase         run script to find proper clk phase       |\n"
	 "| goSep              go  chuck z-axis separation               |\n"
  	 "| setSep             set chuck z-axis separation               |\n"
	 "| goCont             go  chuck z-axis contact                  |\n"
 	 "| setCont            set chuck z-axis contact                  |\n"
	 "| goL                go  chuck Load position                   |\n"
 	 "| setL               set chuck Load position                   |\n"
 	 "| goH                go  chuck Home position                   |\n"
  	 "| setH               set chuck Home position                   |\n"
	 "| first              go to first die (not for std wafer test)  |\n"
	 "| next               go to next die  (not for std wafer test)  |\n"
	 "| goto <y> <x> <L>   go to die       (not for std wafer test)  |\n"
 	 "| createmap          create sorted roc coordinates file        |\n"
	 "| pr <command>       send command to prober (see Alessi manual)|\n"
	 "+--------------------------------------------------------------+\n",
	 stdout);
	}
	else if (settings.proberPort == -2) //---new to manual Alessi (no rs232 communication)
	{
	 fputs("\n"
	 "+-- control commands ------------------------------------------+\n"
	 "| h                  display this text                         |\n"
	 "| exit               exit commander                            |\n"
	 "+-- chip test -------------------------------------------------+\n"
	 "| test <y x ABCD posx(mm) posy(mm)>    - wafer test on Alessi  |\n"
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

