// defectlist.cpp

#include "defectlist.h"


bool CDefectList::add(int x, int y)
{
	if (n>=DEFLISTSIZE) return false;

	for (int i=0; i<n; i++)
		if ( x == list[i].x && y == list[i].y)
			return true;

	list[n].x = x;
	list[n].y = y;
	n++;

	return true;
}
