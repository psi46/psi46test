/*---------------------------------------------------------------------
 *
 *  filename:    pixel_dtb.h
 *
 *  description: PSI46 testboard API for DTB
 *	author:      Beat Meier
 *	date:        1.12.2015
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

	bool RpcLink(bool verbose = true);

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


	// === DTB identification ===============================================

	RPC_EXPORT void GetInfo(stringR &info);
	RPC_EXPORT uint16_t GetBoardId();	// reads the board number

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


	// === Clock, Timing ====================================================

	RPC_EXPORT void cDelay(uint16_t clocks);
	RPC_EXPORT void uDelay(uint16_t us);
	void mDelay(uint16_t ms);


	// --- select ROC/Module clock source
	#define CLK_SRC_INT  0
	#define CLK_SRC_EXT  1
	RPC_EXPORT void SetClockSource(uint8_t source);

	// --- check if external clock is present
	RPC_EXPORT bool IsClockPresent();

	// --- set clock clock frequency (clock divider)
	#define MHZ_1_25   5
	#define MHZ_2_5    4
	#define MHZ_5      3
	#define MHZ_10     2
	#define MHZ_20     1
	#define MHZ_40     0  // default
	RPC_EXPORT void SetClock(uint8_t MHz);

	// --- set clock stretch
	// width = 0 -> disable stretch
	#define STRETCH_AFTER_TRG  0
	#define STRETCH_AFTER_CAL  1
	#define STRETCH_AFTER_RES  2
	#define STRETCH_AFTER_SYNC 3
	RPC_EXPORT void SetClockStretch(uint8_t src,
		uint16_t delay, uint16_t width);

//	RPC_EXPORT void PllReset();
	void SetClock_(unsigned char MHz);

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
	RPC_EXPORT void Sig_SetRdaToutDelay(uint8_t delay);


	// --- digital signal probe ---------------------------------------------
	#define PROBE_OFF            0
	#define PROBE_CLK            1
	#define PROBE_SDA            2
	#define PROBE_SDA_SEND       3
	#define PROBE_PG_TOK         4
	#define PROBE_PG_TRG         5
	#define PROBE_PG_CAL         6
	#define PROBE_PG_RES_ROC     7
	#define PROBE_PG_RES_TBM     8
	#define PROBE_PG_SYNC        9
	#define PROBE_CTR           10
	#define PROBE_TIN           11
	#define PROBE_TOUT          12
	#define PROBE_CLK_PRESEN    13
	#define PROBE_CLK_GOOD      14
	#define PROBE_DAQ0_WR       15
	#define PROBE_CRC           16
	#define PROBE_ADC_RUNNING   19
	#define PROBE_ADC_RUN       20
	#define PROBE_ADC_PGATE     21
	#define PROBE_ADC_START     22
	#define PROBE_ADC_SGATE     23
	#define PROBE_ADC_S         24
	#define PROBE_DS_GATE       29

	RPC_EXPORT void SignalProbeD1(uint8_t signal);
	RPC_EXPORT void SignalProbeD2(uint8_t signal);


	// DESER400
	#define PROBE_FRAME_ERROR    0
	#define PROBE_CODE_ERROR     1
	#define PROBE_ERROR          2  // FRAME or CODE

	#define PROBE_A_HEADER       3
	#define PROBE_A_PACKET       4
	#define PROBE_A_TBM_HDR      5
	#define PROBE_A_ROC_HDR      6
	#define PROBE_A_TBM_TRL      7
	#define PROBE_A_IDLE_ERROR   8
	#define PROBE_A_HDR_ERROR    9

	#define PROBE_B_HEADER      10
	#define PROBE_B_PACKET      11
	#define PROBE_B_TBM_HDR     12
	#define PROBE_B_ROC_HDR     13
	#define PROBE_B_TBM_TRL     14
	#define PROBE_B_IDLE_ERROR  15
	#define PROBE_B_HDR_ERROR   16

	RPC_EXPORT void SignalProbeDeserD1(uint8_t deser, uint8_t signal);
	RPC_EXPORT void SignalProbeDeserD2(uint8_t deser, uint8_t signal);


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
	RPC_EXPORT void Pon();
	RPC_EXPORT void Poff();

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

    RPC_EXPORT uint16_t _GetVD_Cap();
    RPC_EXPORT uint16_t _GetVD_Reg();
    RPC_EXPORT uint16_t _GetVDAC_Reg();

    double GetVD_Cap(){ return _GetVD_Cap()/1000.0;}
    double GetVD_Reg(){ return _GetVD_Reg()/1000.0;}
    double GetVDAC_Reg(){ return _GetVDAC_Reg()/1000.0;}


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
	RPC_EXPORT void Pg_SetCmdAll(vector<uint16_t> &cmd);
	RPC_EXPORT void Pg_SetSum(uint16_t delays);
	RPC_EXPORT void Pg_Stop();
	RPC_EXPORT void Pg_Single();
	RPC_EXPORT void Pg_Trigger();
	RPC_EXPORT void Pg_Triggers(uint32_t triggers, uint16_t period);
	RPC_EXPORT void Pg_Loop(uint16_t period);


	// --- Trigger ----------------------------------------------------------
	#define TRG_SEL_ASYNC      0x0100
	#define TRG_SEL_SYNC       0x0080
	#define TRG_SEL_SINGLE     0x0040
	#define TRG_SEL_GEN        0x0020
	#define TRG_SEL_PG         0x0010

	#define TRG_SEL_ASYNC_DIR  0x0800
	#define TRG_SEL_SYNC_DIR   0x0400
	#define TRG_SEL_SINGLE_DIR 0x0008
	#define TRG_SEL_GEN_DIR    0x0200
	#define TRG_SEL_PG_DIR     0x0004

	#define TRG_SEL_CHAIN      0x0002
	#define TRG_SEL_SYNC_OUT   0x0001

	RPC_EXPORT void Trigger_Select(uint16_t mask);

	RPC_EXPORT void Trigger_Delay(uint8_t delay);

	RPC_EXPORT void Trigger_Timeout(uint16_t timeout);

	RPC_EXPORT void Trigger_SetGenPeriodic(uint32_t periode);
	RPC_EXPORT void Trigger_SetGenRandom(uint32_t rate);

	#define TRG_SEND_SYN   1
	#define TRG_SEND_TRG   2
	#define TRG_SEND_RSR   4
	#define TRG_SEND_RST   8
	#define TRG_SEND_CAL  16
	RPC_EXPORT void Trigger_Send( uint8_t send);

	// --- data aquisition --------------------------------------------------
	RPC_EXPORT uint32_t Daq_Open(uint32_t buffersize = 10000000, uint8_t channel = 0);
	RPC_EXPORT void Daq_Close(uint8_t channel = 0);
	RPC_EXPORT void Daq_Start(uint8_t channel = 0);
	RPC_EXPORT void Daq_Stop(uint8_t channel = 0);
	RPC_EXPORT void Daq_MemReset(uint8_t channel = 0);
	RPC_EXPORT uint32_t Daq_GetSize(uint8_t channel = 0);
	RPC_EXPORT uint8_t Daq_FillLevel(uint8_t channel);
	RPC_EXPORT uint8_t Daq_FillLevel();

	// --- Read data from DAQ FIFO
	// Meaning if return bits
#define DAQ_FIFO_OVFL 4
#define DAQ_MEM_OVFL  2
#define DAQ_STOPPED   1

	RPC_EXPORT uint8_t Daq_Read(HWvectorR<uint16_t> &data,
			uint32_t blocksize = 65536, uint8_t channel = 0);

	RPC_EXPORT uint8_t Daq_Read(HWvectorR<uint16_t> &data,
			uint32_t blocksize, uint32_t &availsize, uint8_t channel = 0);

	void Daq_Read_DeleteData(uint32_t daq_base, int32_t rp);

	RPC_EXPORT void Daq_Select_ADC(uint16_t blocksize, uint8_t source,
			uint8_t start, uint8_t stop = 0);

	RPC_EXPORT void Daq_Select_Deser160(uint8_t shift);

	RPC_EXPORT void Daq_Select_Deser400();
	RPC_EXPORT void Daq_Deser400_Reset(uint8_t reset = 3);
	RPC_EXPORT void Daq_Deser400_OldFormat(bool old);

	RPC_EXPORT void Daq_Select_Datagenerator(uint16_t startvalue);

	RPC_EXPORT void Daq_DeselectAll();

	// --- DESER400 configuration -------------------------------------------
	RPC_EXPORT void Deser400_Enable(uint8_t deser);
	RPC_EXPORT void Deser400_Disable(uint8_t deser);
	RPC_EXPORT void Deser400_DisableAll();

	RPC_EXPORT void Deser400_SetPhase(uint8_t deser, uint8_t phase);
	RPC_EXPORT void Deser400_SetPhaseAuto(uint8_t deser);
	RPC_EXPORT void Deser400_SetPhaseAutoAll();

	RPC_EXPORT uint8_t Deser400_GetXor(uint8_t deser);
	RPC_EXPORT uint8_t Deser400_GetPhase(uint8_t deser);


	/* --- deser400 phase detector trigger

		period     rate    / measure time:
		  0       112.5 ns /   75 ns
		  1       212.5 ns /  175 ns
		  2       412.5 ns /  375 ns
		  3       812.5 ns /  775 ns

		width parameter no longer used
	*/
	RPC_EXPORT void Deser400_GateRun(uint8_t width, uint8_t period);
	RPC_EXPORT void Deser400_GateSingle(uint8_t width); // not used
	RPC_EXPORT void Deser400_GateStop(); // not used


	// --- ROC/module Communication -----------------------------------------
	// -- set the i2c address for the following commands
	RPC_EXPORT void roc_I2cAddr(uint8_t id);
	RPC_EXPORT void roc_I2cAddr_Layer_1(uint8_t id);

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

	// -- enable/disable all double columns
	RPC_EXPORT void roc_AllCol_Enable(bool on);

	// -- mask all pixels of a column and the corresponding double column
	RPC_EXPORT void roc_Col_Mask(uint8_t col);

	// -- mask all pixels and columns of the chip
	RPC_EXPORT void roc_Chip_Mask();

	// --- TBM
	RPC_EXPORT bool TBM_Present();

	// --- TBM/module Communication -----------------------------------------
	RPC_EXPORT void tbm_Enable(bool on);

	RPC_EXPORT void tbm_Addr(uint8_t hub, uint8_t port);

	RPC_EXPORT void mod_Addr(uint8_t hub);
	RPC_EXPORT void mod_Addr(uint8_t hub0, uint8_t hub1); // set switch for layer 1 and additional hubid

	RPC_EXPORT void tbm_Set(uint8_t reg, uint8_t value);

	RPC_EXPORT bool tbm_Get(uint8_t reg, uint8_t &value);

	RPC_EXPORT bool tbm_GetRaw(uint8_t reg, uint32_t &value);

	// --- Wafer test functions
	RPC_EXPORT bool TestColPixel(uint8_t col, uint8_t trimbit, bool sensor_cal, vectorR<uint8_t> &res);

	// Ethernet test functions
	RPC_EXPORT void Ethernet_Send(string &message);
	RPC_EXPORT uint32_t Ethernet_RecvPackets();

	RPC_EXPORT void VectorTest(vector<uint16_t> &in, vectorR<uint16_t> &out);

	// --- Temperature Measurement ------------------------------------------
	RPC_EXPORT uint16_t GetADC(uint8_t addr);
};
