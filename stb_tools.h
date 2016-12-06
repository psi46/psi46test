// stb_tools.h

#pragma once

//#include "psi46test.h"
// #include "datastream.h"


namespace stb
{


template <class T>
class Value
{
	bool valid;
	T value;
public:
	Value() : valid(false) {}
	bool IsValid() { return valid; };
	void Invalidate() { valid = false; }
	T& Get() { return value; }
	void Put(T& x) { value = x; valid = true; }
	T operator=(T x) { Put(x); return x; }
	operator const T&() { return value; }
};




// Module Configuration
struct MC
{
	// STB adapter
	enum { ADP_L12=0, ADP_L3=1, ADP_L4=2, ADP_NO=3 } adapter;

	int layer; // 0 = single module
	enum 
	{
		SDATA_4321=0, SDATA_43=1, SDATA_21=2, SDATA_4=3, SDATA_3=4, SDATA_2=5, SDATA_1=6,
		SDATA_L1=7, SDATA_L2=8, SDATA_L34=9 // single module
	} sdata;
	unsigned int nSdata;
	int powerGrp; // -1 = direct connection (module adapter)
	int powerGrpSize; // module per power group
	int hvGrp;
	enum {TBM8, TBM9, TBM10} tbm;
	enum {PSI46DIG, PROC600} roc;
	int moduleConnector;
	bool NeedsSTB() const { return layer != 0; }
};


#define MODTYPE_L1  39
#define MODTYPE_L2  40
#define MODTYPE_L34 41

class CModType
{
	int sel;
	static const MC MODCONF[42];
public:
	CModType() : sel(0) {}
	CModType(int select) : sel(select) {}
	CModType operator= (int select) { sel = select; return *this; }
	const MC& Get() { return MODCONF[sel];  }
	int GetSel() { return sel; }
	void SetHubAddr(int hub);
	void SetRocAddr(int roc);
};



class CPower
{
	bool singleMode; // only one power group on
	bool PGOn[6];

	static const double VD_MODULE; // V
	static const double VD_SLOPE;  // V/module
	static const double ID_0;      // A
	static const double ID_MODULE; // A/module

	static const double VA_MODULE; // V
	static const double VA_SLOPE;  // V/module
	static const double IA_0;      // A
	static const double IA_MODULE; // A/module

	static const double VD_CB;  // V
	static const double ID_CB;  // A

public:
	CPower();
	void PowerSave(bool on) { singleMode = on; }

	void ModPon(CModType mod, int msDelay = 400);
	void ModPoff(CModType mod);
	void ModPoffAll();
	void CbPon();
	void CbPoff();

	double GetVD(CModType mod);
	double GetID(CModType mod);
	double GetVA(CModType mod);
	double GetIA(CModType mod);
	double GetVC();
	double GetIC();
};





class CDataChannel
{
	bool isOpen;
	int channel;
	CDataPipe<CDataRecord*, CEvent*> *dec;

	CDtbSource src;
	CDataRecordScannerMODD raw;
	CModDigDecoder decDig;
	CModDigLinearDecoder decLin;
	CAnalyzer dummy;
	CSink<CEvent*> pump;

	CAnalyzer *a_first;
	bool rawPipeInserted;
public:
	CDataChannel() : isOpen(false), channel(0), dec(&decDig) {}
//	CDataChannel(int ch) { Open(ch); }
	~CDataChannel() { Close(); }
	void Open(CModType roc, int ch);
	void AddPipe(CAnalyzer &a);
	void AddPipe(CDataPipe<CDataRecord*> &a);
	void RemoveAllPipes();
	void Close() { src.Close(); }
	int GetChannel() { return channel; }
	void Enable() { src.Enable(); }
	void Disable() { src.Disable(); }
	void GetAll();
};


class CDataProcessing
{
	unsigned int nChannels;
	CDataChannel c[8];
	CModType m;
	CAnalyzer* debug_event[8];
public:
	CDataProcessing();
	CDataProcessing(CModType mod);
	~CDataProcessing() { Close(); }

	void Open(CModType mod);
	void AddPipe(unsigned int channel, CAnalyzer &a);
	void AddPipe(unsigned int channel, CDataPipe<CDataRecord*> &a);
	void RemoveAllPipes();
	void Close();

	int GetDeser(int channel) { return c[channel].GetChannel()/2; }
	int GetDeserChannel(int channel) { return c[channel].GetChannel(); }

	bool GetVsdata(unsigned int sdata, double &vp, double &vn);

	void Enable()  { for (unsigned int i=0; i<nChannels; i++) c[i].Enable(); }
	void Disable() { for (unsigned int i=0; i<nChannels; i++) c[i].Disable(); }
	void GetAll()  { for (unsigned int i=0; i<nChannels; i++) c[i].GetAll(); }

	void SendReset();
	void SendTrigger(unsigned int count);

	void TakeData(unsigned int nTrigger);

	// debug methods
	void EnableLogging(const char *prefix);
};



class CSignalQuality : public CAnalyzer
{
protected:
	CEvent* Read();

	void Add(unsigned int xor);
public:
	unsigned int n;
	unsigned int histo[8];

	double quality;
	double phase;

	CSignalQuality() { Clear(); }
	void CalculateQP();
	void Clear();
	void Print();
};



/* Data Format
   A3 A2 A1 A0 | C3 C2 C1 C0 | D7 D6 D5 D4 | D3 D2 D1 D0
   A[3:0] : ROC Address
   C[3:0] : Command sent by roc_SetDAC(255, C)
   D[7:0] : Data Value
     C =  0: Last Data
	 C =  1: Last Address
	 C =  2: Last pixel col
	 C =  3: Last pixel row
	 C =  8: Vd
	 C =  9: Va
	 C = 10: vana
	 C = 11: vbg
	 C = 12: iana
*/
class CRdbValue
{
protected:
	int value;
private:
	bool start;
	unsigned short shift;
public:
	CRdbValue() : value(-1), start(false), shift(0) {}
	void Clear() { value = -1; start = false; shift = 0; }
	void Add(unsigned int v);
	friend class CReadback;
};


class CReadback : public CAnalyzer
{
	FILE *f;
	CRdbValue rdb[8];
protected:
	CEvent* Read();
public:
	CReadback() : f(0) {}
	~CReadback() { if (f) fclose(f); }
	void Logging(const char *filename);
	int operator[] (unsigned int i) { return rdb[i].value; }
	void Clear() { for (int i=0; i<8; i++) rdb[i].Clear(); }
	void Print();
};


} // namespace stb
