/* -------------------------------------------------------------
 *
 *  file:        cmd_stb.h
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

// =======================================================================
//  chip/wafer test commands
// =======================================================================

HELP_CAT("stb")

CMD_REG(stbena, "", "force STB present")
CMD_REG(stbdis, "", "force STB not present")

CMD_REG(spresent,"", "Check if STB present")
CMD_REG(sflashw,"<string>", "Write string to flash memory")
CMD_REG(sflashr,"", "Read flash memory")
CMD_REG(sgetadapter, "", "Identify connected adapter board")

CMD_REG(spon, "<srcs>", "STB <srcs> power on")
CMD_REG(spoff, "<srcs>", "STB <srcs> power off")
CMD_REG(sva, "<srcs> <mV>", "STB set <srcs> VA in mV")
CMD_REG(svd, "<srcs> <mV>", "STB set <srcs> VD in mV")
CMD_REG(sia, "<srcs> <mA>", "STB set <srcs> IA in mA")
CMD_REG(sid, "<srcs> <mA>", "STB set <srcs> ID in mA")
CMD_REG(sgetva, "<src>", "STB get <src> VA in V")
CMD_REG(sgetvd, "<src>", "STB get <src> VD in V")
CMD_REG(sgetia, "<src>", "STB get <src> IA in mA")
CMD_REG(sgetid, "<src>", "STB get <src> ID in mA")
CMD_REG(shvon, "<channels>", "STB switch HV channels on")
CMD_REG(shvoff, "<channels>", "STB switch HV channels off")
CMD_REG(sselsdata, "<ch>", "STB select sdata channel")
CMD_REG(sgetsdata, "<ch> [<pn>]", "Read sdata voltage level")

CMD_REG(st, "<sector name>", "Sector Test")
