// pixelmap.cpp

#include "profiler.h"
#include "pixelmap.h"
#include "chipdatabase.h"


void CPixelMap::Init()
{
	int col, row;
	mapExist = pulseHeightExist = levelExist = false;
	pulseHeight1Exist = pulseHeight2Exist = false;
	for (col=0; col<ROCNUMCOLS; col++) for (row=0; row<ROCNUMROWS; row++)
	{
		map[col][row] = 0; // dead pixel
		pulseHeight[col][row] = 0;
		pulseHeight1[col][row] = 0;
		pulseHeight2[col][row] = 0;
		refLevel[col][row] = 0;
		level[col][row][0] = 0;
		level[col][row][1] = 0;
		level[col][row][2] = 0;
		level[col][row][3] = 0;
	}
}


void CPixelMap::SetMaskedCount(unsigned int x, unsigned int y, unsigned int count)
{ PROFILING
	if (IsInRange(x,y))
	{
		map[x][y] = (map[x][y] & ~0x00f0) | ((count<<4) & 0x00f0);
	}
}


void CPixelMap::SetUnmaskedCount(unsigned int x, unsigned int y, unsigned int count)
{ PROFILING
	if (IsInRange(x,y))
	{
		map[x][y] = (map[x][y] & ~0x000f) | (count & 0x000f);
	}
}


void CPixelMap::SetDefectTrimBit(unsigned int x, unsigned int y,
	unsigned int bit, bool defect)
{
	if (IsInRange(x,y) && (bit < 4))
	{
		unsigned int mask = 0x0100 << bit;
		if (defect) map[x][y] |= mask;
		else        map[x][y] &= ~mask;
	}
}


void CPixelMap::SetDefectColCode(unsigned int x, unsigned int y, bool defect)
{ PROFILING
	if (IsInRange(x,y))
	{
		if (defect) map[x][y] |=  0x1000;
		else        map[x][y] &= ~0x1000;
	}
}


void CPixelMap::SetDefectRowCode(unsigned int x, unsigned int y, bool defect)
{ PROFILING
	if (IsInRange(x,y))
	{
		if (defect) map[x][y] |=  0x2000;
		else        map[x][y] &= ~0x2000;
	}
}


void CPixelMap::SetPulseHeight(unsigned int x, unsigned int y,
							short value)
{ PROFILING
	if (IsInRange(x,y)) pulseHeight[x][y] = value;
}


void CPixelMap::SetPulseHeight1(unsigned int x, unsigned int y,
							short value)
{
	if (IsInRange(x,y)) pulseHeight1[x][y] = value;
}


void CPixelMap::SetPulseHeight2(unsigned int x, unsigned int y,
							short value)
{
	if (IsInRange(x,y)) pulseHeight2[x][y] = value;
}


void CPixelMap::SetRefLevel(unsigned int x, unsigned int y,
							unsigned char value)
{
	if (IsInRange(x,y)) refLevel[x][y] = value;
}


void CPixelMap::SetLevel(unsigned int x, unsigned int y,
		unsigned char bit, unsigned char value)
{
	if (IsInRange(x,y) && bit<4) level[x][y][bit] = value;
}



unsigned int CPixelMap::GetMaskedCount(unsigned int x, unsigned int y)
{
	return (IsInRange(x,y)) ? (map[x][y]>>4) & 0x000f : 0;
}


unsigned int CPixelMap::GetUnmaskedCount(unsigned int x, unsigned int y)
{
	return (IsInRange(x,y)) ? map[x][y] & 0x000f : 0;
}


bool CPixelMap::GetDefectReadoutCnts(unsigned int x, unsigned int y)
{
	if (!IsInRange(x,y)) return false;
	return (map[x][y] & 0x00ff) != 1;
}


bool CPixelMap::GetDefectTrimBit(unsigned int x, unsigned int y,
	unsigned int bit)
{
	if (!IsInRange(x,y) || bit >= 4) return false;

	unsigned int mask = 0x0100 << bit;
	return (map[x][y] & mask) != 0;
}


bool CPixelMap::GetDefectTrimBit(unsigned int x, unsigned int y)
{
	if (!IsInRange(x,y)) return false;

	return (map[x][y] & 0x0f00) != 0;
}


bool CPixelMap::GetDefectColCode(unsigned int x, unsigned int y)
{
	if (!IsInRange(x,y)) return false;

	return (map[x][y] & 0x1000) != 0;

}


bool CPixelMap::GetDefectRowCode(unsigned int x, unsigned int y)
{
	if (!IsInRange(x,y)) return false;

	return (map[x][y] & 0x2000) != 0;

}


bool CPixelMap::GetDefectAddrCode(unsigned int x, unsigned int y)
{
	if (!IsInRange(x,y)) return false;

	return (map[x][y] & 0x3000) != 0;

}


short CPixelMap::GetPulseHeight(unsigned int x, unsigned int y)
{
	return IsInRange(x,y) ? pulseHeight[x][y] : 0;
}


short CPixelMap::GetPulseHeight1(unsigned int x, unsigned int y)
{
	return IsInRange(x,y) ? pulseHeight1[x][y] : 0;
}


short CPixelMap::GetPulseHeight2(unsigned int x, unsigned int y)
{
	return IsInRange(x,y) ? pulseHeight2[x][y] : 0;
}


void CPixelMap::UpdateTrimDefects()
{
	int col, row;

	for (col=0; col<ROCNUMCOLS; col++)	for (row=0; row<ROCNUMROWS; row++)
	{
		bool def;
//		def = refLevel[col][row] - level[col][row][0] <= 2;
//		SetDefectTrimBit(col,row,0,def);

//		def = refLevel[col][row] - level[col][row][1] <= 2;
//		SetDefectTrimBit(col,row,1,def);

		def = refLevel[col][row] - level[col][row][2] <= 2;
		SetDefectTrimBit(col,row,2,def);

		def = refLevel[col][row] - level[col][row][3] <= 2;
		SetDefectTrimBit(col,row,3,def);
	}
}


bool CPixelMap::IsDefect(unsigned int x, unsigned int y)
{
	if (!IsInRange(x,y)) return false;
	return map[x][y] != 1;
}


unsigned int CPixelMap::DefectPixelCount()
{
	int col, row;
	unsigned int cnt = 0;

	for (col=0; col<ROCNUMCOLS; col++)	for (row=0; row<ROCNUMROWS; row++)
		if (map[col][row] != 1) cnt++;

	return cnt;
}


bool CPixelMap::Hex(char **s, unsigned int &value)
{
	unsigned int x = 0;
	if (**s == ' ') (*s)++;
	for (int i=0; i<4; i++)
	{
		if (**s == '.') { value = 1; (*s)+=4; return true; }
		else if ('0' <= **s && **s <= '9') x = 16*x + **s - '0';
		else if ('A' <= **s && **s <= 'F') x = 16*x + **s - 'A' + 10;
		else return false;
		(*s)++;
	}
	value = x;
	return true;
}


bool CPixelMap::ReadPixMap(CScanner &Log)
{
	int row, col;
	char *s = Log.getNextLine();

	if (strlen(s) == 0)
	{
		for (col=0; col<52; col++) for (row=0; row<80; row++)
			map[col][row] = 1;
		mapExist = true;
		return true;
	}

	for (row=79; row>=0; row--)
	{
		for (col=0; col<52; col++)
			if (!Hex(&s, map[col][row])) return false;
		s = Log.getNextLine();
	}

	mapExist = true;
	return true;
}


bool CPixelMap::ReadPulseHeight(CScanner &Log)
{
	int row, col, value;

	for (row=79; row>=0; row--)
	{
		char *s = Log.getNextLine();
		for (col=0; col<52; col++)
		{
			if (sscanf(s, "%i", &value) != 1) return false;
			pulseHeight[col][row] = short(value);
			s+=5;
		}
	}

	pulseHeightExist = true;
	return true;
}


bool CPixelMap::ReadPulseHeight1(CScanner &Log)
{
	int row, col, value;

	for (row=79; row>=0; row--)
	{
		char *s = Log.getNextLine();
		for (col=0; col<52; col++)
		{
			if (s[1] == '.') pulseHeight1[col][row] = 10000;
			else if (sscanf(s, "%i", &value) != 1) return false;
			else pulseHeight1[col][row] = short(value);
			s+=5;
		}
	}

	pulseHeight1Exist = true;
	return true;
}


bool CPixelMap::ReadPulseHeight2(CScanner &Log)
{
	int row, col, value;

	for (row=79; row>=0; row--)
	{
		char *s = Log.getNextLine();
		for (col=0; col<52; col++)
		{
			if (s[1] == '.') pulseHeight2[col][row] = 10000;
			else if (sscanf(s, "%i", &value) != 1) return false;
			else pulseHeight2[col][row] = short(value);
			s+=5;
		}
	}

	pulseHeight2Exist = true;
	return true;
}


bool CPixelMap::Dec(char **s, unsigned char &value)
{
	unsigned int x = 0;
	if (**s == ' ') (*s)++;
	for (int i=0; i<2; i++)
	{
		if (**s == 'O') { value = 100; (*s)+=2; return true; }
		else if ('0' <= **s && **s <= '9') x = 10*x + **s - '0';
		else if ((i == 0) && (**s == ' '));
		else return false;
		(*s)++;
	}
	value = x;
	return true;
}


bool CPixelMap::ReadRefLevel(CScanner &Log)
{
	int col, row;
	for (row=79; row>=0; row--)
	{
		char *s = Log.getNextLine();
		for (col=0; col<52; col++)
			if (!Dec(&s, refLevel[col][row])) return false;
	}

	return true;
}


bool CPixelMap::ReadLevel(CScanner &Log, unsigned int trimbit)
{
	int col, row;
	for (row=79; row>=0; row--)
	{
		char *s = Log.getNextLine();
		for (col=0; col<52; col++)
			if (!Dec(&s, level[col][row][trimbit]))
			{
				printf("col %i row %i\n", row, col);
				return false;
			}
	}

	return true;
}

/*
void CPixelMap::Histo(CHistogram &h)
{
	int col, row;
	h.clear();
	for (col=0; col<52; col++) for (row=0; row<80; row++)
		h.add(refLevel[col][row]);
}


void CPixelMap::HistoDiff(unsigned int trimbit, CHistogram &h)
{
	int col, row;
	h.clear();
	for (col=0; col<52; col++) for (row=0; row<80; row++)
		h.add(refLevel[col][row] - level[col][row][trimbit]);
}
*/


void CPixelMap::Print(CProtocol &prot)
{
	int col, row;
	for (row=ROCNUMROWS-1; row>=0; row--)
	{
		for (col=0; col<ROCNUMCOLS; col++)
			if (map[col][row] == 1) prot.printf(" ....");
			else                    prot.printf(" %04X", map[col][row]);
		prot.puts("\n");
	}

}


void CPixelMap::PrintPulseHeight(CProtocol &prot)
{
	int col, row;
	for (row=ROCNUMROWS-1; row>=0; row--)
	{
		for (col=0; col<ROCNUMCOLS; col++)
		{
			int ph = pulseHeight[col][row];
			prot.printf(" %4i", ph);
		}
		prot.puts("\n");
	}

}


void CPixelMap::PrintPulseHeight1(CProtocol &prot)
{
	if (!pulseHeight1Exist) return;

	int col, row;

	prot.section("PH1");
	for (row=ROCNUMROWS-1; row>=0; row--)
	{
		for (col=0; col<ROCNUMCOLS; col++)
		{
			int value = GetPulseHeight1(col,row);
			if (value < -990) prot.puts(" ....");
			else prot.printf(" %4i", value);
		}
		prot.puts("\n");
	}
}


void CPixelMap::PrintPulseHeight2(CProtocol &prot)
{
	if (!pulseHeight2Exist) return;

	int col, row;

	prot.section("PH2");
	for (row=ROCNUMROWS-1; row>=0; row--)
	{
		for (col=0; col<ROCNUMCOLS; col++)
		{
			int value = GetPulseHeight2(col,row);
			if (value < -990) prot.puts(" ....");
			else prot.printf(" %4i", value);
		}
		prot.puts("\n");
	}
}


void CPixelMap::PrintRefLevel(CProtocol &prot)
{
	int col, row;
	char s[ROCNUMCOLS*4+20];

	for (row=ROCNUMROWS-1; row>=0; row--)
	{
		for (col=0; col<ROCNUMCOLS; col++)
		{
			unsigned int lvl = GetRefLevel(col,row);
			if (lvl >= 200)
				sprintf(s+4*col, "  OR");
			else
				sprintf(s+4*col, "%4u", lvl);
		}
		prot.puts(s);
		prot.puts("\n");
	}
}


void CPixelMap::PrintLevel(unsigned int trimbit, CProtocol &prot)
{
	int col, row;
	char s[ROCNUMCOLS*4+20];

	for (row=ROCNUMROWS-1; row>=0; row--)
	{
		for (col=0; col<ROCNUMCOLS; col++)
		{
			int lvl = /* int(GetRefLevel(col,row)) - */ GetLevel(col,row,trimbit);
			if (lvl >= 250)
				sprintf(s+4*col, "  OR");
			else
				sprintf(s+4*col, "%4i", lvl);
		}
		prot.puts(s);
		prot.puts("\n");
	}
}


