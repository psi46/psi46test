// histo.h

#ifndef HISTO_H
#define HIST_H

#include <stdio.h>
#include "protocol.h"


class CHistogram
{
	int m_min;
	int m_max;
	int m_bin;
	unsigned int m_total, m_integral;

	int n;
	unsigned int *m_data;
	bool FindPeak(int &x, int &p);
	void ScaleX(CProtocol &f);
	void LogScaleY(int y, CProtocol &f);
public:
	CHistogram(int min, int max, int bin);
	~CHistogram() { delete[] m_data; }
	void Clear();
	void AddData(int value);
	unsigned int Peak();
	void ScanPeaks(int data[], int size, int &count);
	void Print(CProtocol &f, unsigned int linesize = 1);
	void Plot(CProtocol &f, unsigned int scale);
	void LogPlot(CProtocol &f);
};


#endif
