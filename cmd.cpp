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
	 "| initdiced          initialize chip wafer test after dicing   |\n" //---new
	 "| testdiced          run chip test after dicing                |\n" //---new
 	 "| createmap          create sorted roc coordinates file        |\n" //---new
	 "| test               run chip test                             |\n"
	 "| pr <command>       send command to prober                    |\n"
	 "| sep                prober z-axis separation                  |\n"
	 "| contact            prober z-axis contact                     |\n"
	 "| firs               go to first die-not for entire wafer test |\n"
	 "| next               go to next die-not for entire wafer test  |\n"
	 "| goto <x> <y>       go to die-not for entire wafer test       |\n"
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
	 "| test <x y ABCD posx(mm) posy(mm)>    - wafer test on Alessi  |\n"
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

