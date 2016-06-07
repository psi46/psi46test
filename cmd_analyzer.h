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
CMD_REG(phscan, "<x> <y> <cal range>", "pulse height scan")


// === DROC600 test ======================================================
HELP_CAT("droc600")
CMD_REG(scanaddr, "", "address scan")
CMD_REG(scanphxy, "<range x> <range y> [<count>]", "scan pulse heigth")
CMD_REG(scanph, "<count>", "scan pulse heigth repeated")
CMD_REG(dbmatch, "<count>", "DB analog cell uniformity")
CMD_REG(dbmatch2, "<count>", "DB analog cell uniformity (2nd methode)")
CMD_REG(evenodd, "<x> <y>", "pulse height even odd effect")
CMD_REG(multiread, "<n readouts>", "multiple readout")
CMD_REG(cluster, "<n readouts>", "cluster test")
CMD_REG(cluster2, "<n readouts>", "cluster test")
CMD_REG(db1, "", "Data buffer test")
CMD_REG(enapx, "{<x> <y>}", "enables n pixels and calibrate")

CMD_REG(rotest0, "", "DB test 0")
CMD_REG(rotest1, "", "DB test 1")
CMD_REG(rotest2, "", "DB test 2")
CMD_REG(rotest3, "", "DB test 3")
CMD_REG(idslope, "", "Id vs Pixel Rate")