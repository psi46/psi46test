// analyzer.h

#pragma once

#include "pixel_dtb.h"
#include "datastream.h"
#include <vector>
// #include <stdint.h>


struct PixelReadoutData
{
	unsigned int hdr;
	int n;  // # pixel
	int x;  // x of first pixel
	int y;  // y of first pixel
	int p;  // pulse heigth
	void Clear() { hdr = 0; n = x = y = p = 0; }
};


void DumpData(const vector<uint16_t> &x, unsigned int n);

void DecodePixel(const std::vector<uint16_t> &x, int &pos, PixelReadoutData &pix);



// ==========================================================================

class CReadback : public CAnalyzer
{
	bool valid;
	unsigned int data;
	void (*alert)(unsigned int);
	CRocEvent* Read();
public:
	CReadback() : valid(0), data(0), alert(0) {}
	bool IsValid() { return valid; }
	unsigned int GetData() { if (valid) { valid = 0; return data; } return 0; }
	unsigned int SetCallback(void (*callback)(unsigned int)) { alert = callback; }
};


class CPulseHeight : public CAnalyzer
{
	CRocEvent* Read();
};