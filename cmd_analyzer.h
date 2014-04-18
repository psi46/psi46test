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


CMD_REG(takedata, "", "")
CMD_REG(takedata2, "", "")
CMD_REG(showclk, "", "")
CMD_REG(showctr, "", "")
CMD_REG(showsda, "", "")
CMD_REG(dselmod, "", "")
CMD_REG(dmodres, "", "")
CMD_REG(dselroc, "", "")
CMD_REG(dselroca, "", "")
CMD_REG(dselsim, "", "")
CMD_REG(dseloff, "", "")
CMD_REG(decoding, "", "")


// =======================================================================
//  experimential ROC test commands
// =======================================================================

CMD_REG(daqtest, "", "test DAQ read function")
CMD_REG(analyze, "", "test analyzer chain")
CMD_REG(analyzeana, "", "test analyzer chain")
CMD_REG(adcsingle, "", "ADC problem test 3")
CMD_REG(adcpeak, "", "ADC problem test 2")
CMD_REG(adchisto, "", "ADC problem test 1")
CMD_REG(adctransfer, "", "")
CMD_REG(adctest, "", "check ADC pulse height readout")
CMD_REG(ethsend, "<string>", "send <string> in a Ethernet packet")
CMD_REG(ethrx, "", "shows number of received packets")
CMD_REG(shmoo, "", "shmoo vx xrange vy ymin yrange")
// CMD_REG(phscan, "", "")
CMD_REG(deser160, "", "allign deser160")
// CMD_REG(readback, "", "")
