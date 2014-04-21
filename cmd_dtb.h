/* -------------------------------------------------------------
 *
 *  file:        cmd_dtb.cpp
 *
 *  description: command line interpreter
 *               DTB base functions
 *
 *  author:      Beat Meier
 *  modified:    21.4.2014
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */


// =======================================================================
//  connection, communication, startup commands
// =======================================================================

CMD_REG(scan, "", "Get infos of all connected DTBs")
CMD_REG(open, "[<name>]", "open a DTB (with name)")
CMD_REG(close, "", "close DTB connection")
CMD_REG(rpclink, "", "link all DTB functions")
CMD_REG(welcome, "", "blink with LEDs")
CMD_REG(setled, "<mask>", "set atb LEDs")
CMD_REG(log, "<text>", "writes text to log file")
CMD_REG(upgrade, "<filename>", "upgrade DTB")
CMD_REG(rpcinfo, "", "list all DTB functions")
CMD_REG(info, "", "show detailed DTB info")
CMD_REG(ver, "", "shows DTB software version number")
CMD_REG(version, "", "shows DTB software version")
CMD_REG(boardid, "", "get board id")
CMD_REG(init, "", "inits the testboard")
CMD_REG(flush, "", "flushes usb buffer")
CMD_REG(clear, "", "clears usb data buffer")


// =======================================================================
//  delay commands
// =======================================================================

CMD_REG(udelay, "<us>", "waits <us> microseconds")
CMD_REG(mdelay, "<ms>", "waits <ms> milliseconds")


// =======================================================================
//  test board commands
// =======================================================================

CMD_REG(clksrc, "<source>", "Select clock source")
CMD_REG(clkok, "clkok", "Check if ext clock is present")
CMD_REG(fsel, "<freqdiv>", "clock frequency select")
CMD_REG(stretch, "<src> <delay> <width>", "stretch clock")
CMD_REG(clk, "<delay>", "clk delay")
CMD_REG(sda, "<delay>", "sda delay")
// CMD_REG(rda, "", "")
CMD_REG(ctr, "<delay>", "ctr delay")
CMD_REG(tin, "<delay>", "tin delay")
CMD_REG(clklvl, "<level>", "clk signal level")
CMD_REG(sdalvl, "<level>", "sda signel level")
CMD_REG(ctrlvl, "<level>", "ctr signel level")
CMD_REG(tinlvl, "<level>", "tin signel level")
CMD_REG(clkmode, "<mode>", "clk mode")
CMD_REG(sdamode, "<mode>", "sda mode")
CMD_REG(ctrmode, "<mode>", "ctr mode")
CMD_REG(tinmode, "<mode>", "tin mode")
CMD_REG(sigoffset, "<offset>", "output signal offset")
CMD_REG(lvds, "", "LVDS inputs")
CMD_REG(lcds, "", "LCDS inputs")
// CMD_REG(tout, "", "")
// CMD_REG(trigout, "", "")
CMD_REG(pon, "", "switch ROC power on")
CMD_REG(poff, "", "switch ROC power off")
CMD_REG(va, "<mV>", "set VA in mV")
CMD_REG(vd, "<mV>", "set VD in mV")
CMD_REG(ia, "<mA>", "set IA in mA")
CMD_REG(id, "<mA>", "set ID in mA")
CMD_REG(getva, "", "get VA in V")
CMD_REG(getvd, "", "get VD in V")
CMD_REG(getia, "", "get IA in mA")
CMD_REG(getid, "", "get ID in mA")
CMD_REG(hvon, "", "switch HV on")
CMD_REG(hvoff, "", "switch HV off")
CMD_REG(reson, "", "activate reset")
CMD_REG(resoff, "", "deactivate reset")
CMD_REG(status, "", "shows testboard status")
CMD_REG(rocaddr, "", "set ROC address")
CMD_REG(d1, "<signal>", "assign signal to D1 output")
CMD_REG(d2, "<signal>", "assign signal to D2 outout")
CMD_REG(a1, "<signal>", "assign analog signal to A1 output")
CMD_REG(a2, "<signal>", "assign analog signal to A2 outout")
CMD_REG(probeadc, "<signal>", "assign analog signal to ADC")
CMD_REG(pgset, "<addr> <bits> <delay>", "set pattern generator entry")
CMD_REG(pgstop, "", "stops pattern generator")
CMD_REG(pgsingle, "", "send single pattern")
CMD_REG(pgtrig, "", "enable external pattern trigger")
CMD_REG(pgloop, "<period>", "start patterngenerator in loop mode")

// === DAQ ==================================================================

CMD_REG(dopen, "<buffer size> [<ch>]", "Open DAQ and allocate memory")
CMD_REG(dclose, "[<channel>]", "Close DAQ")
CMD_REG(dstart, "[<channel>]", "Enable DAQ")
CMD_REG(dstop, "[<channel>]", "Disable DAQ")
CMD_REG(dsize, "[<channel>]", "Show DAQ buffer fill state")
CMD_REG(dread, "[<channel>]", "Read Daq buffer and show as raw data")
CMD_REG(dreadr, "[<channel>]", "Read Daq buffer and show as ROC data")
CMD_REG(dreadm, "[<channel>]", "Read Daq buffer and show as module data")
CMD_REG(dreada, "[<channel>]", "Read Daq buffer and show as raw data")
CMD_REG(dselmod, "", "select deser400 for DAQ channel 0");
CMD_REG(dmodres, "", "reset all deser400");
CMD_REG(dselroc, "<value>", "select deser160 for DAQ channel 0");
CMD_REG(dselroca, "<value>", "select adc for channel 0");
CMD_REG(dselsim, "<startvalue>", "select data generator for channel 0");
CMD_REG(dseloff, "", "deselect all");


// =======================================================================
//  tbm commands
// =======================================================================

CMD_REG(tbmdis, "", "disable TBM")
CMD_REG(tbmsel, "<hub> <port>", "set hub and port address")
CMD_REG(modsel, "<hub>", "set hub address for module")
CMD_REG(tbmset, "<reg> <value>", "set TBM register")

/*
CMD_REG(tbmget, "", "")
CMD_REG(tbmgetraw, "", "")
CMD_REG(tbmregs, "", "")
CMD_REG(modscan, "", "")
*/

CMD_REG(select, "<addr range>", "set i2c address")
CMD_REG(dac, "<address> <value>", "set DAC")
CMD_REG(vana, "<value>", "set Vana")
CMD_REG(vtrim, "<value>", "set Vtrim")
CMD_REG(vthr, "<value>", "set VthrComp")
CMD_REG(vcal, "<value>", "set Vcal")
CMD_REG(wbc, "<value>", "set WBC")
CMD_REG(ctl, "<value>", "set control register")
CMD_REG(cole, "<range>", "enable column")
CMD_REG(cold, "<range>", "disable columns")
CMD_REG(pixe, "<range> <range> <value>", "trim pixel")
CMD_REG(pixd, "<range> <range>", "kill pixel")
CMD_REG(cal, "<range> <range>", "calibrate pixel")
CMD_REG(cals, "<range> <range>", "sensor calibrate pixel")
CMD_REG(cald, "", "clear calibrate")
CMD_REG(mask, "", "mask all pixel and cols")
