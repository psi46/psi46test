/* -------------------------------------------------------------
 *
 *  file:        cmd_analyzer.cpp
 *
 *  description: command line interpreter
 *               experimental function
 *
 *  author:      Beat Meier
 *  modified:    21.4.2014
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */


HELP_CAT("ext")

CMD_REG(showclk, "", "")
CMD_REG(showctr, "", "")
CMD_REG(showsda, "", "")
CMD_REG(decoding, "", "")


// =======================================================================
//  experimential ROC test commands
// =======================================================================

CMD_REG(daqtest, "", "test DAQ read function")
CMD_REG(analyze, "", "test analyzer chain")
CMD_REG(ethsend, "<string>", "send <string> in a Ethernet packet")
CMD_REG(ethrx, "", "shows number of received packets")
CMD_REG(shmoo, "", "shmoo vx xrange vy ymin yrange")
CMD_REG(deser160, "", "allign deser160")
// CMD_REG(readback, "", "")
