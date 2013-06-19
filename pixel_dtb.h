/*---------------------------------------------------------------------
 *
 *  filename:    pixel_dtb.h
 *
 *  description: PSI46 testboard API for DTB
 *	author:      Beat Meier
 *	date:        22.4.2013
 *	rev:
 *
 *---------------------------------------------------------------------
 */

#pragma once

#include "rpc.h"
//#include "pipe.h"
#include "linux/usb.h"

// size of ROC pixel array
#define ROC_NUMROWS  80  // # rows
#define ROC_NUMCOLS  52  // # columns
#define ROC_NUMDCOLS 26  // # double columns (= columns/2)

// delay cells
#define SIGNAL_CLK      8
#define SIGNAL_SDA      9
#define SIGNAL_CTR     10
#define SIGNAL_TIN     11
#define SIGNAL_TOUT    12
#define SIGNAL_RDA     12
#define SIGNAL_TRGOUT  13

// clock frequenz settings
#define MHZ_1_25   5
#define MHZ_2_5    4
#define MHZ_5      3
#define MHZ_10     2
#define MHZ_20     1
#define MHZ_40     0

#define PIXMASK  0x80

// PUC register addresses for roc_SetDAC
#define	Vdig        0x01
#define Vana        0x02
#define	Vsh         0x03
#define	Vcomp       0x04
#define	Vleak_comp  0x05
#define	VrgPr       0x06
#define	VwllPr      0x07
#define	VrgSh       0x08
#define	VwllSh      0x09
#define	VhldDel     0x0A
#define	Vtrim       0x0B
#define	VthrComp    0x0C
#define	VIBias_Bus  0x0D
#define	Vbias_sf    0x0E
#define	VoffsetOp   0x0F
#define	VIbiasOp    0x10
#define	VoffsetRO   0x11
#define	VIon        0x12
#define	VIbias_PH   0x13
#define	Ibias_DAC   0x14
#define	VIbias_roc  0x15
#define	VIColOr     0x16
#define	Vnpix       0x17
#define	VsumCol     0x18
#define	Vcal        0x19
#define	CalDel      0x1A
#define	RangeTemp   0x1B
#define	WBC         0xFE
#define	CtrlReg     0xFD

// sources for clock stretch trigger
#define STRETCH_AFTER_TIN  0
#define STRETCH_AFTER_TRG  1
#define STRETCH_AFTER_CAL  2
#define STRETCH_AFTER_RES  3

// constants for signal force command
#define OVW_CLK   0x10
#define OVW_SDA   0x20
#define OVW_CTR   0x40
#define OVW_TIN   0x80
#define SET_CLK   0x01
#define SET_SDA   0x02
#define SET_CTR   0x04
#define SET_TIN   0x08


class CTestboard
{
	CRpcIo *rpc_io;
//	CPipeClient pipe;
	CUSB usb;
public:
	CRpcIo& GetIo() { return *rpc_io; }

	CTestboard() : rpc_io(&usb) {}
	~CTestboard() {}


	// === DTB connection methods ==============================================

//	bool OpenPipe(const char *name) { return pipe.Open(name); }
//	void ClosePipe() { pipe.Close(); }

	bool EnumFirst(unsigned int &nDevices) { return usb.EnumFirst(nDevices); };
	bool EnumNext(char name[]) { return usb.EnumNext(name); }
	bool Open(char name[], bool init=true); // opens a connection
	void Close();				// closes the connection to the testboard
	bool IsConnected() { return usb.Connected(); }
	const char * ConnectionError()
	{ return usb.GetErrorMsg(usb.GetLastError()); }

	void Flush() { rpc_io->Flush(); }
	void Clear() { rpc_io->Clear(); }


	// === DTB identification ==================================================

	bool CheckCompatibility() { return GetLocalRpcVersion() == GetDtbRpcVersion(); }
	bool CheckUserCompatibility() { return GetLocalRpcUserVersion() == GetDtbRpcUserVersion(); }
	uint64_t GetLocalRpcVersion() { return rpc_MainVersion; }
	uint64_t GetLocalRpcUserVersion() { return rpc_UserVersion; }
	DTB_EXPORT(id) uint64_t GetDtbRpcVersion();
	DTB_EXPORT(id) uint64_t GetDtbRpcUserVersion();
	DTB_EXPORT(id) void GetDtbRpcTimestamp(stringR &ts);

	DTB_EXPORT(id) uint16_t GetVersion();
	DTB_EXPORT(id) void GetVersionString(stringR &x);
	DTB_EXPORT(id) void GetComment(stringR &x);
	DTB_EXPORT(id) uint8_t GetBoardId();	// reads the board number


	// === DTB service =========================================================

	DTB_EXPORT(service) uint16_t GetServiceVersion();
	DTB_EXPORT(service) void Welcome();
	DTB_EXPORT(service) void SetLed(uint8_t x);

	DTB_EXPORT(service) uint32_t UpgradeStart();
	DTB_EXPORT(service) uint32_t UpgradeData(uint32_t seq, string &data);
	DTB_EXPORT(service) uint32_t UpgradeAbort();
	DTB_EXPORT(service) uint32_t UpgradeExecute(uint32_t lastSeq, uint32_t errorNr);

	//	DTB_EXPORT(service) void Bootstrap();

	// === DTB1 functions ======================================================

	DTB_EXPORT(dtb1) uint16_t GetDTB1Version();
	DTB_EXPORT(dtb1) void Init();


	// === Clock, Timing ====================================================

	DTB_EXPORT(dtb1) void cDelay(uint16_t clocks);
	DTB_EXPORT(dtb1) void uDelay(uint16_t us);
	void mDelay(uint16_t ms);

//	DTB_EXPORT(dtb1) unsigned char isClockPresent();
//	DTB_EXPORT(dtb1) void SetClock(unsigned char MHz);
//	DTB_EXPORT(dtb1) void SetClockStretch(unsigned char src,
//		unsigned short delay, unsigned short width);
//	DTB_EXPORT(dtb1) void SetDelay(unsigned char signal, unsigned short ns);


	// === Signal Delay =========================================================

	#define SIG_CLK 0
	#define SIG_CTR 1
	#define SIG_SDA 2
	#define SIG_TIN 3

	#define SIG_MODE_NORMAL  0
	#define SIG_MODE_LO      1
	#define SIG_MODE_HI      2

	DTB_EXPORT(dtb1) void Sig_SetMode(uint8_t signal, uint8_t mode);
	DTB_EXPORT(dtb1) void Sig_SetPRBS(uint8_t signal, uint8_t speed);
	DTB_EXPORT(dtb1) void Sig_SetDelay(uint8_t signal, uint16_t delay, int8_t duty = 0);
	DTB_EXPORT(dtb1) void Sig_SetLevel(uint8_t signal, uint8_t level);
	DTB_EXPORT(dtb1) void Sig_SetOffset(uint8_t offset);
	DTB_EXPORT(dtb1) void Sig_SetLVDS();
	DTB_EXPORT(dtb1) void Sig_SetLCDS();

	// === digital signal probe =============================================

	#define PROBE_OFF     0
	#define PROBE_CLK     1
	#define PROBE_SDA     2
	#define PROBE_PGTOK   3
	#define PROBE_PGTRG   4
	#define PROBE_PGCAL   5
	#define PROBE_PGRESR  6
	#define PROBE_PGREST  7
	#define PROBE_PGSYNC  8
	#define PROBE_CTR     9
	#define PROBE_CLKP   10
	#define PROBE_CLKG   11
	#define PROBE_CRC    12

	DTB_EXPORT(dtb1) void SignalProbeD1(uint8_t signal);
	DTB_EXPORT(dtb1) void SignalProbeD2(uint8_t signal);


	// === analog signal probe =================================================

	#define PROBEA_TIN     0
	#define PROBEA_SDATA1  1
	#define PROBEA_SDATA2  2
	#define PROBEA_CTR     3
	#define PROBEA_CLK     4
	#define PROBEA_SDA     5
	#define PROBEA_TOUT    6
	#define PROBEA_OFF     7

	#define GAIN_1   0
	#define GAIN_2   1
	#define GAIN_3   2
	#define GAIN_4   3

	DTB_EXPORT(dtb1) void SignalProbeA1(uint8_t signal);
	DTB_EXPORT(dtb1) void SignalProbeA2(uint8_t signal);
	DTB_EXPORT(dtb1) void SignalProbeADC(uint8_t signal, uint8_t gain = 0);


	// === ROC/Module power VD/VA ===========================================

	DTB_EXPORT(dtb1) void Pon();	// switch ROC power on
	DTB_EXPORT(dtb1) void Poff();	// switch ROC power off

	DTB_EXPORT(dtb1) void _SetVD(uint16_t mV);
	DTB_EXPORT(dtb1) void _SetVA(uint16_t mV);
	DTB_EXPORT(dtb1) void _SetID(uint16_t uA100);
	DTB_EXPORT(dtb1) void _SetIA(uint16_t uA100);

	DTB_EXPORT(dtb1) uint16_t _GetVD();
	DTB_EXPORT(dtb1) uint16_t _GetVA();
	DTB_EXPORT(dtb1) uint16_t _GetID();
	DTB_EXPORT(dtb1) uint16_t _GetIA();

	void SetVA(double V) { _SetVA(uint16_t(V*1000)); }  // set VA voltage
	void SetVD(double V) { _SetVD(uint16_t(V*1000)); }  // set VD voltage
	void SetIA(double A) { _SetIA(uint16_t(A*10000)); }  // set VA current limit
	void SetID(double A) { _SetID(uint16_t(A*10000)); }  // set VD current limit

	double GetVA() { return _GetVA()/1000.0; }   // get VA voltage in V
	double GetVD() { return _GetVD()/1000.0; }	 // get VD voltage in V
	double GetIA() { return _GetIA()/10000.0; }  // get VA current in A
	double GetID() { return _GetID()/10000.0; }  // get VD current in A

	DTB_EXPORT(dtb1) void HVon();
	DTB_EXPORT(dtb1) void HVoff();
	DTB_EXPORT(dtb1) void ResetOn();
	DTB_EXPORT(dtb1) void ResetOff();
	DTB_EXPORT(dtb1) uint8_t GetStatus();
	DTB_EXPORT(dtb1) void SetRocAddress(uint8_t addr);


	// == pulse pattern generator ==============================================

	#define PG_TOK   0x0100
	#define PG_TRG   0x0200
	#define PG_CAL   0x0400
	#define PG_RESR  0x0800
	#define PG_REST  0x1000
	#define PG_SYNC  0x2000

	DTB_EXPORT(dtb1) void Pg_SetCmd(uint16_t addr, uint16_t cmd);
//	DTB_EXPORT(dtb1) void Pg_SetCmdAll(vector<uint16_t> &cmd);
	DTB_EXPORT(dtb1) void Pg_Disable();
	DTB_EXPORT(dtb1) void Pg_Single();
	DTB_EXPORT(dtb1) void Pg_Trigger();
	DTB_EXPORT(dtb1) void Pg_Loop(uint16_t period);


	// ======================================================================


//	DTB_EXPORT(dtb1) void HVon();	// switch HV relais on
//	DTB_EXPORT(dtb1) void HVoff();	// switch HV relais off

//	DTB_EXPORT(dtb1) void ResetOn();	// switch RESET-line to reset state (low)
//	DTB_EXPORT(dtb1) void ResetOff();	// switch RESET-line to not reset state (high)


//	DTB_EXPORT(dtb1) void ForceSignal(unsigned char pattern);

//	DTB_EXPORT(dtb1) void I2cAddr(unsigned char id);		// set testboard I2C address

	// === DTB2 functions ======================================================
	DTB_EXPORT(dtb2) uint16_t GetDTB2Version();

	// --- Wafer Test Adapter --------------------------------------------------

//	DTB_EXPORT(dtb2) unsigned short _GetVD_Reg();    // regulated VD
//	DTB_EXPORT(dtb2) unsigned short _GetVD_CAP();    // unregulated VD for contact test
//	DTB_EXPORT(dtb2) unsigned short _GetVDAC_CAP();  // regulated VDAC

//	double GetVD_Reg()   { return _GetVD_Reg()/1000.0; }    // regulated VD
//	double GetVD_CAP()   { return _GetVD_CAP()/1000.0; }    // unregulated VD for contact test
//	double GetVDAC_CAP() { return _GetVDAC_CAP()/1000.0; }  // regulated VDAC

	// === ROC/module Communication ============================================
	DTB_EXPORT(roc) uint16_t GetRocVersion();

	// -- set the i2c address for the following commands
	DTB_EXPORT(roc) void roc_I2cAddr(uint8_t id);

	// -- sends "ClrCal" command to ROC
	DTB_EXPORT(roc) void roc_ClrCal();

	// -- sets a single (DAC) register
	DTB_EXPORT(roc) void roc_SetDAC(uint8_t reg, uint8_t value);

	// -- set pixel bits (count <= 60)
	//    M - - - 8 4 2 1
	DTB_EXPORT(roc) void roc_Pix(uint8_t col, uint8_t row, uint8_t value);

	// -- trimm a single pixel (count < =60)
	DTB_EXPORT(roc) void roc_Pix_Trim(uint8_t col, uint8_t row, uint8_t value);

	// -- mask a single pixel (count <= 60)
	DTB_EXPORT(roc) void roc_Pix_Mask(uint8_t col, uint8_t row);

	// -- set calibrate at specific column and row
	DTB_EXPORT(roc) void roc_Pix_Cal(uint8_t col, uint8_t row, bool sensor_cal = false);

	// -- enable/disable a double column
	DTB_EXPORT(roc) void roc_Col_Enable(uint8_t col, bool on);

	// -- mask all pixels of a column and the coresponding double column
	DTB_EXPORT(roc) void roc_Col_Mask(uint8_t col);

	// -- mask all pixels and columns of the chip
	DTB_EXPORT(roc) void roc_Chip_Mask();


	// --- TBM -----------------------------------------------------------------

//	DTB_EXPORT(roc) bool TBMPresent();

//	DTB_EXPORT(roc) void tbm_Enable(bool on);

//	DTB_EXPORT(roc) void tbm_Addr(unsigned char hub, unsigned char port);

//	DTB_EXPORT(roc) void mod_Addr(unsigned char hub);

//	DTB_EXPORT(roc) void tbm_Set(unsigned char reg, unsigned char value);

//	DTB_EXPORT(roc) bool tbm_Get(unsigned char reg, unsigned char &value);

//	DTB_EXPORT(roc) bool tbm_GetRaw(unsigned char reg, long &value);

	// === User methods ========================================================
	DTB_EXPORT(user1) uint16_t GetUser1Version();

	DTB_EXPORT(user1) bool Daq_Open(uint32_t buffersize = 10000000); // max # of samples
	DTB_EXPORT(user1) void Daq_Close();
	DTB_EXPORT(user1) void Daq_Clear();
	DTB_EXPORT(user1) void Daq_Start();
	DTB_EXPORT(user1) void Daq_Stop();
	DTB_EXPORT(user1) uint32_t Daq_GetSize();
	DTB_EXPORT(user1) void Daq_GetData(vectorR<uint16_t> &data, uint16_t maxsize);

	DTB_EXPORT(user1) void Daq_ADC(uint16_t datasize);
	DTB_EXPORT(user1) void Daq_Deser160(bool enable, uint8_t shift);
};
