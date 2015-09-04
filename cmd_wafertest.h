/* -------------------------------------------------------------
 *
 *  file:        cmd_wafertest.cpp
 *
 *  description: command line interpreter
 *               wafer test functions
 *
 *  author:      Beat Meier
 *  modified:    21.4.2014
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

// =======================================================================
//  chip/wafer test commands
// =======================================================================

HELP_CAT("test")
CMD_REG(roctype, "ana|dig", "choose ROC type for test")
CMD_REG(pr, "<command>", "send command to prober")
CMD_REG(goSep, "", "chuck z-axis separation")
CMD_REG(setSep, "", "set chuck z-axis separation")
CMD_REG(goCont, "", "go  chuck z-axis contact")
CMD_REG(setCont, "", "set chuck z-axis contact")
CMD_REG(goL, "", "go  chuck Load position")
CMD_REG(setL, "", "set chuck Load position")
CMD_REG(goH, "", "go  chuck Home position")
CMD_REG(setH, "", "set chuck Home position")
CMD_REG(test, "<chip id>", "run chip test")
CMD_REG(initdiced,"initdiced", "to initialize chip-wafer-test-after-dicing"); //---new
CMD_REG(testdiced,   "", "to run chip-test-after-dicing");  //---new
CMD_REG(createmap, "", "to create sorted coordinates file from previous tests") //---new
CMD_REG(first, "", "go to first die - not for complete wafer test")
CMD_REG(next, "", "go to next die - not for complete wafer test")
CMD_REG(goto, "", "go to specified die - not for complete wafer test")


// -- Wafer Test Adapter commands ----------------------------------------
/*
CMD_REG(vdreg, "", "")    // regulated VD
CMD_REG(vdcap, "", "")    // unregulated VD for contact test
CMD_REG(vdac, "", "")     // regulated VDAC
*/
