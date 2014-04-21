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

CMD_REG(roctype, "ana|dig", "choose ROC type for test")
CMD_REG(pr, "<command>", "send command to prober")
CMD_REG(sep, "", "prober z-axis separation")
CMD_REG(contact, "", "prober z-axis contact")
CMD_REG(test, "<chip id>", "run chip test")
CMD_REG(chippos, "<ABCD>", "move to chip A, B, C or D")
CMD_REG(go, "init|cont", "start wafer test (press <cr> to stop)")
CMD_REG(first, "", "go to first die and clear wafer map")
CMD_REG(next, "", "go to next die")
CMD_REG(goto, "", "go to specified die")

// -- Wafer Test Adapter commands ----------------------------------------
/*
CMD_REG(vdreg, "", "")    // regulated VD
CMD_REG(vdcap, "", "")    // unregulated VD for contact test
CMD_REG(vdac, "", "")     // regulated VDAC
*/
