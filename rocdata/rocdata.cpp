#include "rocdata.h"
#include <cmath>

bool CRocData::Read(unsigned int size, const short *buffer)
{
    header = 0;
	nPixel = 0;

	// --- read header -------------------------------------------------
	if (size < 3) return false;
	header = *(buffer++) & 0xf;
	header = (header << 4) + (*(buffer++) & 0xf);
	header = (header << 4) + (*(buffer++) & 0xf);
    if ((header & 0xff8) != 0x7f8) return false;
	size -= 3;

	// --- read pixel -----------------------------------------------
	while (size != 0 && nPixel < (MAXPIXEL-1))
	{
	  if (size < 6) return false;
	  unsigned int data =  (*(buffer++) & 0xf);
	  data = (data << 4) + (*(buffer++) & 0xf);
	  data = (data << 4) + (*(buffer++) & 0xf);
	  data = (data << 4) + (*(buffer++) & 0xf);
	  data = (data << 4) + (*(buffer++) & 0xf);
	  data = (data << 4) + (*(buffer++) & 0xf);
	  size -= 6;

	  pix[nPixel].value = (data & 0x0f) + ((data >> 1) & 0xf0);
	  data >>= 9;

	  data ^= 0x1ff; // inverted pixel address correction
	  int c =    (data >> 12) & 7;
	  c = c*6 + ((data >>  9) & 7);
	  int r =    (data >>  6) & 7;
	  r = r*6 + ((data >>  3) & 7);
	  r = r*6 + ( data        & 7);
	  pix[nPixel].y = 80 - r/2;
	  pix[nPixel].x = 2*c + (r&1);

	  nPixel++;
	}

	return true;
}


void CRocData::Print(FILE *f)
{
	fprintf(f, " %03X %3u:", header, nPixel);
	for (unsigned int i=0; i<nPixel; i++)
	{
		if (i>0 && (i % 5) == 0) fprintf(f, "\n         ");
		pix[i].Print(f);
	}
}


void CPixData::Print(FILE *f)
{
	fprintf(f, " [%2u %2u %3u]", x, y, value);
}