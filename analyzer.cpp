// analyzer.cpp

#include "analyzer.h"

// --- event lister ---------------------------------------------------------

class CStore : public CAnalyzer
{
	CRocEvent* Read();
};


CRocEvent* CStore::Read()
{
	CRocEvent *data = Get();
	printf("%8u: %03X %4u:\n", data->eventNr, (unsigned int)(data->header), (unsigned int)(data->pixel.size()));
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
