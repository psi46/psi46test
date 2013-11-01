// ps.cpp

#include <stdarg.h>
#include "ps.h"


bool CPostscript::open(char filename[])
{
	if (isOpen()) return false;
	psf = fopen(filename, "wt");
	nPages=0;
	return isOpen();
}


void CPostscript::printf(const char fmt[], ...)
{
	if (!isOpen()) return;
	va_list va;
	va_start(va,fmt);
	vfprintf(psf, fmt, va);
	va_end(va);
}


bool CPostscript::putTempl(const char filename[])
{
	if (!isOpen()) return false;
	char *buffer = new char[1024];
	FILE *tf = fopen(filename, "rt");
	if (tf==NULL) { delete[] buffer; return false; }

	size_t count;
	do
	{
		count = fread(buffer,1,1024,tf);
		if (ferror(psf)) { fclose(tf); delete[] buffer; return false; }
		if (count>0) fwrite(buffer,1,count,psf);
	} while (!feof(tf));
	fclose(tf);
	delete[] buffer;
	return true;
}


void CPostscript::addPage()
{
	if (!isOpen()) return;
	nPages++;
	fprintf(psf,"%%%%Page: %i %i\n", nPages, nPages);	
}


void CPostscript::addTrailer()
{
	if (!isOpen()) return;
	fprintf(psf,"%%%%Trailer\n%%%%Pages: %i\n%%%%EOF\n", nPages);
}
