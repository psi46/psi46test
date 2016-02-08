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
CMD_REG(daqerrorcheckm, "<channel> <period>", "read data from module and list errors")
CMD_REG(desergatescan, "", "xor for different gate length")
CMD_REG(daqreadm, "<channel> <period>", "read, decode and list continous data stream from module with trig gen")
CMD_REG(daqreadt, "<period>", "read, decode and list continous data stream from roc with trig gen")
CMD_REG(analyze, "", "test analyzer chain")
CMD_REG(ethsend, "<string>", "send <string> in a Ethernet packet")
CMD_REG(ethrx, "", "shows number of received packets")
CMD_REG(shmoo, "", "shmoo vx xrange vy ymin yrange")
CMD_REG(deser160, "", "align deser160")
// CMD_REG(readback, "", "")
CMD_REG(phscan, "", "pulse height scan")


// === DROC600 test ======================================================
HELP_CAT("droc600")
CMD_REG(addrscan, "", "address scan");
CMD_REG(multiread, "<n readouts>", "multiple readout");
CMD_REG(cluster, "<n readouts>", "cluster test");
CMD_REG(cluster2, "<n readouts>", "cluster test");
