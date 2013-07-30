// defectlist.h

#ifndef DEFECTLIST_H
#define DEFECTLIST_H


#define DEFLISTSIZE 64


class CDefectList
{
	int n;
	struct { int x, y; } list[DEFLISTSIZE];
public:
	CDefectList() { clear(); }
	
	void clear() { n = 0; }

	int size() { return n; }
	
	bool add(int x, int y);
	
	bool get(int i, int &x, int &y)
	{
		if (i>=n) return false;
		x = list[i].x; y = list[i].y;
		return true;
	}
};

#endif
