// ps.h

#ifndef PS_H
#define PS_H

#include <stdio.h>


class CPostscript
{
  int nPages;
  FILE *psf;
  bool isOpen() { return psf != NULL; }
  void addTrailer();
public:
  bool open(char filename[]);
  void close() { if (isOpen()) { addTrailer(); fclose(psf); psf=NULL; } }
  CPostscript() { psf=NULL; nPages=0; }
  ~CPostscript() { close(); }
  void puts(char s[]) { if (isOpen()) fputs(s,psf); }
  void printf(char fmt[], ...);
  bool putTempl(char filename[]);
  void addPage();
  int getPageNr() { return nPages; }
};


#endif
