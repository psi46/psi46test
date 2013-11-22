/*---------------------------------------------------------------------
 *
 *  filename:    pixel_dtb.h
 *
 *  description: PSI46 testboard API for DTB
 *	author:      Beat Meier
 *	date:        15.7.2013
 *	rev:
 *
 *---------------------------------------------------------------------
 */

#pragma once

#include "profiler.h"

#include "rpc.h"
#include "config.h"

#ifdef _WIN32
#include "pipe.h"
#endif

#include "usb.h"

// size of ROC pixel array
#define ROC_NUMROWS  80  // # rows
#define ROC_NUMCOLS  52  // # columns
#define ROC_NUMDCOLS 26  // # double columns (= columns/2)

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



class CTestboard
{
	RPC_DEFS
	RPC_THREAD

#ifdef _WIN32
	CPipeClient pipe;
#endif
	CUSB usb;

public:
	CRpcIo& GetIo() { return *rpc_io; }

	CTestboard() { RPC_INIT rpc_io = &usb; }
	~CTestboard() { RPC_EXIT }


	// === RPC ==============================================================

	// Don't change the following two entries
	RPC_EXPORT uint16_t GetRpcVersion();
	RPC_EXPORT int32_t  GetRpcCallId(string &callName);

	RPC_EXPORT void GetRpcTimestamp(stringR &ts);

	RPC_EXPORT int32_t GetRpcCallCount();
	RPC_EXPORT bool    GetRpcCallName(int32_t id, stringR &callName);

	// === DTB connection ====================================================

	bool EnumFirst(unsigned int &nDevices) { return usb.EnumFirst(nDevices); };
	bool EnumNext(string &name);
	bool Enum(unsigned int pos, string &name);

	bool FindDTB(string &usbId);
	bool Open(string &name, bool init=true); // opens a connection
	void Close();				// closes the connection to the testboard

#ifdef _WIN32
	bool OpenPipe(const char *name) { return pipe.Open(name); }
	void ClosePipe() { pipe.Close(); }
#endif

	bool IsConnected() { return usb.Connected(); }
	const char * ConnectionError()
	{ return usb.GetErrorMsg(usb.GetLastError()); }

	void Flush() { rpc_io->Flush(); }
	void Clear() { rpc_io->Clear(); }


	// === DTB identification ================================================

	RPC_EXPORT void GetInfo(stringR &info);
	RPC_EXPORT uint16_t GetBoardId();
	RPC_EXPORT void GetHWVersion(stringR &version);
	RPC_EXPORT uint16_t GetFWVersion();
	RPC_EXPORT uint16_t GetSWVersion();


	// === DTB service ======================================================

	// --- upgrade
	RPC_EXPORT uint16_t UpgradeGetVersion();
	RPC_EXPORT uint8_t  UpgradeStart(uint16_t version);
	RPC_EXPORT uint8_t  UpgradeData(string &record);
	RPC_EXPORT uint8_t  UpgradeError();
	RPC_EXPORT void     UpgradeErrorMsg(stringR &msg);
	RPC_EXPORT void     UpgradeExec(uint16_t recordCount);


	// === DTB functions ====================================================

	RPC_EXPORT void Init();

	RPC_EXPORT void Welcome();
	RPC_EXPORT void SetLed(uint8_t x);


	// --- Clock, Timing ----------------------------------------------------
	RPC_EXPORT void cDelay(uint16_t clocks);
	RPC_EXPORT void uDelay(uint16_t us);
	void mDelay(uint16_t ms);


	// --- Signal Delay -----------------------------------------------------
	#define SIG_CLK 0
	#define SIG_CTR 1
	#define SIG_SDA 2
	#define SIG_TIN 3

	#define SIG_MODE_NORMAL  0
	#define SIG_MODE_LO      1
	#define SIG_MODE_HI      2

	RPC_EXPORT void Sig_SetMode(uint8_t signal, uint8_t mode);
	RPC_EXPORT void Sig_SetPRBS(uint8_t signal, uint8_t speed);
	RPC_EXPORT void Sig_SetDelay(uint8_t signal, uint16_t delay, int8_t duty = 0);
	RPC_EXPORT void Sig_SetLevel(uint8_t signal, uint8_t level);
	RPC_EXPORT void Sig_SetOffset(uint8_t offset);
	RPC_EXPORT void Sig_SetLVDS();
	RPC_EXPORT void Sig_SetLCDS();


	// --- digital signal probe ---------------------------------------------
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

	RPC_EXPORT void SignalProbeD1(uint8_t signal);
	RPC_EXPORT void SignalProbeD2(uint8_t signal);


	// --- analog signal probe ----------------------------------------------
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

	RPC_EXPORT void SignalProbeA1(uint8_t signal);
	RPC_EXPORT void SignalProbeA2(uint8_t signal);
	RPC_EXPORT void SignalProbeADC(uint8_t signal, uint8_t gain = 0);


	// --- ROC/Module power VD/VA -------------------------------------------
	RPC_EXPORT void Pon();	// switch ROC power on
	RPC_EXPORT void Poff();	// switch ROC power off

	RPC_EXPORT void _SetVD(uint16_t mV);
	RPC_EXPORT void _SetVA(uint16_t mV);
	RPC_EXPORT void _SetID(uint16_t uA100);
	RPC_EXPORT void _SetIA(uint16_t uA100);

	RPC_EXPORT uint16_t _GetVD();
	RPC_EXPORT uint16_t _GetVA();
	RPC_EXPORT uint16_t _GetID();
	RPC_EXPORT uint16_t _GetIA();

	void SetVA(double V) { _SetVA(uint16_t(V*1000)); }  // set VA voltage
	void SetVD(double V) { _SetVD(uint16_t(V*1000)); }  // set VD voltage
	void SetIA(double A) { _SetIA(uint16_t(A*10000)); }  // set VA current limit
	void SetID(double A) { _SetID(uint16_t(A*10000)); }  // set VD current limit

	double GetVA() { return _GetVA()/1000.0; }   // get VA voltage in V
	double GetVD() { return _GetVD()/1000.0; }	 // get VD voltage in V
	double GetIA() { return _GetIA()/10000.0; }  // get VA current in A
	double GetID() { return _GetID()/10000.0; }  // get VD current in A

	RPC_EXPORT void HVon();
	RPC_EXPORT void HVoff();
	RPC_EXPORT void ResetOn();
	RPC_EXPORT void ResetOff();
	RPC_EXPORT uint8_t GetStatus();
	RPC_EXPORT void SetRocAddress(uint8_t addr);


	// --- pulse pattern generator ------------------------------------------
	#define PG_TOK   0x0100
	#define PG_TRG   0x0200
	#define PG_CAL   0x0400
	#define PG_RESR  0x0800
	#define PG_REST  0x1000
	#define PG_SYNC  0x2000

	RPC_EXPORT void Pg_SetCmd(uint16_t addr, uint16_t cmd);
//	RPC_EXPORT void Pg_SetCmdAll(vector<uint16_t> &cmd);
	RPC_EXPORT void Pg_Stop();
	RPC_EXPORT void Pg_Single();
	RPC_EXPORT void Pg_Trigger();
	RPC_EXPORT void Pg_Loop(uint16_t period);

//	RPC_EXPORT uint16_t GetUser1Version();


	// --- data aquisition --------------------------------------------------
	RPC_EXPORT uint32_t Daq_Open(uint32_t buffersize = 1000000, uint8_t dma = 0 ); // max # of samples
	RPC_EXPORT void Daq_Close();
	RPC_EXPORT void Daq_Start();
	RPC_EXPORT void Daq_Stop();
	RPC_EXPORT uint32_t Daq_GetSize(uint8_t channel = 0);

	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data,
			 uint8_t channel, uint16_t blocksize = 16384);

	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data,
			uint8_t channel, uint16_t blocksize, uint32_t &availsize);

	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data,
			uint16_t blocksize, uint32_t &availsize);
	
	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data,
			 uint16_t blocksize = 16384);

	RPC_EXPORT void Daq_Select_ADC(uint16_t blocksize, uint8_t source,
			uint8_t start, uint8_t stop = 0);

	RPC_EXPORT void Daq_Select_Deser160(uint8_t shift);


	// --- ROC/module Communication -----------------------------------------
	// -- set the i2c address for the following commands
	RPC_EXPORT void roc_I2cAddr(uint8_t id);

	// -- sends "ClrCal" command to ROC
	RPC_EXPORT void roc_ClrCal();

	// -- sets a single (DAC) register
	RPC_EXPORT void roc_SetDAC(uint8_t reg, uint8_t value);

	// -- set pixel bits (count <= 60)
	//    M - - - 8 4 2 1
	RPC_EXPORT void roc_Pix(uint8_t col, uint8_t row, uint8_t value);

	// -- trimm a single pixel (count < =60)
	RPC_EXPORT void roc_Pix_Trim(uint8_t col, uint8_t row, uint8_t value);

	// -- mask a single pixel (count <= 60)
	RPC_EXPORT void roc_Pix_Mask(uint8_t col, uint8_t row);

	// -- set calibrate at specific column and row
	RPC_EXPORT void roc_Pix_Cal(uint8_t col, uint8_t row, bool sensor_cal = false);

	// -- enable/disable a double column
	RPC_EXPORT void roc_Col_Enable(uint8_t col, bool on);

	// -- mask all pixels of a column and the coresponding double column
	RPC_EXPORT void roc_Col_Mask(uint8_t col);

	// -- mask all pixels and columns of the chip
	RPC_EXPORT void roc_Chip_Mask();

	// --- TBM --------------------------------------------------------------

	// -- Enables the use of a TBM
	RPC_EXPORT void tbm_Enable(bool on);

	// -- Selects hub and port to be used, disables module abstraction layer
	RPC_EXPORT void tbm_Addr(uint8_t hub, uint8_t port);

	// -- Selects hub. Port is selected according to barrel module layout convention
	RPC_EXPORT void mod_Addr(uint8_t hub);

	// -- Sends I2C commands to the selected address
	RPC_EXPORT void tbm_Set(uint8_t reg, uint8_t value);

	// -- Reads back the register via I2C
	RPC_EXPORT bool tbm_Get(uint8_t reg, uint8_t &value);

	// -- Reads back uninterpreted data, gives raw bit pattern. For debugging purposes if tbm_Get failes with error
	RPC_EXPORT bool tbm_GetRaw(uint8_t reg, uint32_t &value);

	// -- Other TBM functions

	RPC_EXPORT void tbm1_Write(uint32_t hubAddr, uint32_t addr, int32_t value);

	RPC_EXPORT void tbm2_Write(uint32_t hubAddr, uint32_t addr, int32_t value);

	RPC_EXPORT void tbm_Write(uint32_t hubAddr, uint32_t addr, int32_t value);



// --- Wafer test functions
	RPC_EXPORT bool testColPixel(uint8_t col, uint8_t trimbit, vectorR<uint8_t> &res);

	// Ethernet test functions
	//RPC_EXPORT void Ethernet_Send(string &message);
	//RPC_EXPORT uint32_t Ethernet_RecvPackets();
};
