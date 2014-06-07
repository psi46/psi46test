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

CMD_REG(showrocdata, "", "")

// =======================================================================
//  experimential ROC test commands
// =======================================================================

CMD_REG(vectortest, "<length>", "send/receive a vector")
CMD_REG(daqtest, "", "test DAQ read function")
CMD_REG(daqtest2, "", "test DAQ read function in continous mode")
CMD_REG(daqreadm, "", "read, decode and list continous data stream from module")

CMD_REG(analyze, "", "test analyzer chain")
CMD_REG(ethsend, "<string>", "send <string> in a Ethernet packet")
CMD_REG(ethrx, "", "shows number of received packets")
CMD_REG(shmoo, "", "shmoo vx xrange vy ymin yrange")
CMD_REG(deser160, "", "align deser160")
// CMD_REG(readback, "", "")
