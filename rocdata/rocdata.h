#pragma once

#include <stdio.h>

class CPixData
{
public:
	unsigned int x, y;
	unsigned int value;
	void Print(FILE *f);
};


#define MAXPIXEL 256

class CRocData
{
public:
  unsigned int header;
  unsigned int nPixel;
  CPixData pix[MAXPIXEL];

  CRocData() : header(0), nPixel(0) {}
  bool Read(unsigned int size, const short *buffer);
  void Print(FILE *f);
};

