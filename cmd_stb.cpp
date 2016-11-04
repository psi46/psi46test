/* -------------------------------------------------------------
 *
 *  file:        cmd_stb.cpp
 *
 *  description: command line interpreter
 *               Sector Test
 *
 *  author:      Beat Meier
 *  modified:    8.9.2016
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#include "cmd.h"
#include "stb.h"

// =======================================================================
//  Sector Test (Board) commands
// =======================================================================

CMD_PROC(stbena)
{
	tb.stb_SetPresent(true);
	DO_FLUSH
}


CMD_PROC(stbdis)
{
	tb.stb_SetPresent(false);
	DO_FLUSH
}


CMD_PROC(spresent)
{
	if (tb.stb_IsPresent())
		printf("STB detected\n");
	else
		printf("STB not detected\n");
}


CMD_PROC(sflashw)
{
	char buffer[256];
	PAR_STRINGEOL(buffer, 255);
	string s = buffer;
	
	if (tb.stb_WriteFlash(s))
		printf("OK\n");
	else
		printf("ERROR\n");
}


CMD_PROC(sflashr)
{
	string s;
	if (tb.stb_ReadFlash(s))
		printf("OK: %s\n", s.c_str());
	else
		printf("ERROR\n");
}


CMD_PROC(sgetadapter)
{
	int id = tb.stb_GetAdapterId();
	printf("STB Adapter %i\n", id);
}


CMD_PROC(spon)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	for (src = src_min; src <= src_max; src++) tb.stb_Pon(src);

	DO_FLUSH
}


CMD_PROC(spoff)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	for (src = src_min; src <= src_max; src++) tb.stb_Poff(src);

	DO_FLUSH
}


CMD_PROC(sva)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	int value;
	PAR_INT(value, 0, 4000);

	for (src = src_min; src <= src_max; src++) tb._stb_SetVA(src, value);
	DO_FLUSH
}


CMD_PROC(svd)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	int value;
	PAR_INT(value, 0, 4000);

	for (src = src_min; src <= src_max; src++) tb._stb_SetVD(src, value);
	DO_FLUSH
}


CMD_PROC(sia)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	int value;
	PAR_INT(value, 0, 3000);

	value *= 10;
	for (src = src_min; src <= src_max; src++) tb._stb_SetIA(src, value);
	DO_FLUSH
}


CMD_PROC(sid)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	int value;
	PAR_INT(value, 0, 3000);

	value *= 10;
	for (src = src_min; src <= src_max; src++) tb._stb_SetID(src, value);
	DO_FLUSH
}


CMD_PROC(sgetva)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	printf("\n");
	for (src = src_min; src <= src_max; src++)
	{
		double v = tb.stb_GetVA(src);
		printf(" VA(%i) = %1.3fV\n", src, v);
	}
}


CMD_PROC(sgetvd)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	printf("\n");
	for (src = src_min; src <= src_max; src++)
	{
		double v = tb.stb_GetVD(src);
		printf(" VD(%i) = %1.3fV\n", src, v);
	}
}


CMD_PROC(sgetia)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	printf("\n");
	for (src = src_min; src <= src_max; src++)
	{
		double i = tb.stb_GetIA(src);
		printf(" IA(%i) = %1.1fmA\n", src, i*1000.0);
	}
}


CMD_PROC(sgetid)
{
	int src, src_min, src_max;
	PAR_RANGE(src_min, src_max, 0, 5)

	printf("\n");
	for (src = src_min; src <= src_max; src++)
	{
		double i = tb.stb_GetID(src);
		printf(" ID(%i) = %1.1fmA\n", src, i*1000.0);
	}
}


CMD_PROC(shvon)
{
	int ch, ch_min, ch_max;
	PAR_RANGE(ch_min, ch_max, 0, 6)

	for (ch = ch_min; ch <= ch_max; ch++) tb.stb_HVon(ch);

	DO_FLUSH
}


CMD_PROC(shvoff)
{
	int ch, ch_min, ch_max;
	PAR_RANGE(ch_min, ch_max, 0, 6)

	for (ch = ch_min; ch <= ch_max; ch++) tb.stb_HVoff(ch);

	DO_FLUSH
}


CMD_PROC(sselsdata)
{
	int ch;
	PAR_INT(ch, 0, 15);

	tb.stb_SetSdata((uint8_t)ch);
	DO_FLUSH
}


CMD_PROC(sgetsdata)
{
	int channel;
	char pol[4];
	bool pos = true, neg = true;

	PAR_INT(channel, 0, 15)

	if (PAR_IS_STRING(pol, 3))
	{
		pos = (strchr(pol, 'p') || strchr(pol, 'P'));
		neg = (strchr(pol, 'n') || strchr(pol, 'N'));
	}

	if (pos && neg)
	{
		int vp = tb._stb_GetVSdata((uint8_t)channel, true);
		int vn = tb._stb_GetVSdata((uint8_t)channel, false);
		printf("V_sdata(%i) p:%4i mV; n:%4i mV\n", channel, vp, vn);
	}
	else if (pos)
	{
		int v = tb._stb_GetVSdata((uint8_t)channel, true);
		printf("V_sdata(%i) p:%4i mV\n", channel, v);
	}
	else if (neg)
	{
		int v = tb._stb_GetVSdata((uint8_t)channel, false);
		printf("V_sdata(%i) n:%4i mV\n", channel, v);
	}
}



CMD_PROC(st)
{
	char name[256];
	PAR_STRINGEOL(name, 254);
	stb::Sectortest(name);
}


CMD_PROC(smodstart)
{
	int id_min, id_max;
	PAR_RANGE(id_min, id_max, 0, 38)
	stb::StartAllModules(id_min, id_max);
}


CMD_PROC(sdefhub)
{
	int hub, id_min, id_max;
	PAR_RANGE(id_min, id_max, 0, 41)
	PAR_INT(hub, -1, 31)
	for (int i=id_min; i<=id_max; i++) stb::SetDefaultHub(i, hub);
}