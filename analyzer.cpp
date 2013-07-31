// analyzer.cpp

#include "analyzer.h"

using namespace std;


void DumpData(const vector<uint16_t> &x, unsigned int n)
{
	unsigned int i;
	printf("\n%i samples\n", int(x.size()));
	for (i=0; i<n && i<x.size(); i++)
	{
		if (x[i] & 0x8000) printf(">"); else printf(" ");
		printf("%03X", (unsigned int)(x[i] & 0xfff));
		if (i%16 == 15) printf("\n");
	}
	printf("\n");
}

void DecodePixel(const vector<uint16_t> &x, int &pos, PixelReadoutData &pix)
{ PROFILING
	pix.Clear();
	unsigned int raw = 0;

	// check header
	if (pos >= int(x.size())) throw int(1); // missing data
	if ((x[pos] & 0x8ffc) != 0x87f8) throw int(2); // wrong header
	pix.hdr = x[pos++] & 0xfff;

	if (pos >= int(x.size()) || (x[pos] & 0x8000)) return; // empty data readout

	// read first pixel
	raw = (x[pos++] & 0xfff) << 12;
	if (pos >= int(x.size()) || (x[pos] & 0x8000)) throw int(3); // incomplete data
	raw += x[pos++] & 0xfff;
	pix.n++;

	// read additional noisy pixel
	int cnt = 0;
	while (!(pos >= int(x.size()) || (x[pos] & 0x8000))) { pos++; cnt++; }
	pix.n += cnt / 2;

	pix.p = (raw & 0x0f) + ((raw >> 1) & 0xf0);
	raw >>= 9;
	int c =    (raw >> 12) & 7;
	c = c*6 + ((raw >>  9) & 7);
	int r =    (raw >>  6) & 7;
	r = r*6 + ((raw >>  3) & 7);
	r = r*6 + ( raw        & 7);
	pix.y = 80 - r/2;
	pix.x = 2*c + (r&1);
}




// --- event lister ---------------------------------------------------------

class CStore : public CAnalyzer
{
	CRocEvent* Read();
};


CRocEvent* CStore::Read()
{
	CRocEvent *data = Get();
	printf("%8u: %03X %4u:\n", (unsigned int)(data->eventNr), (unsigned int)(data->header), (unsigned int)(data->pixel.size()));
	return data;
}


// --- column statistics ----------------------------------------------------

class CColActivity : public CAnalyzer
{
	unsigned long colhits[52];
	CRocEvent* Read();
public:
	CColActivity() { Clear(); }
	void Clear();
};


void CColActivity::Clear()
{
	for (int i=0; i<52; i++) colhits[i] = 0;
}


CRocEvent* CColActivity::Read()
{
	CRocEvent *data = Get();
	list<CPixel>::iterator i;
	for (i = data->pixel.begin(); i != data->pixel.end(); i++)
		if (i->x >= 0 && i->x < 52) colhits[i->x]++;
	return data;
}


void Analyzer(CTestboard &tb)
{
	CBinaryDTBSource src(tb);
	CDataRecordScanner rec;
	CRocDecoder dec;
	CStore lister;
	CSink<CRocEvent*> pump;

	src >> rec >> dec >> lister >> pump;

	pump.GetAll();
}
