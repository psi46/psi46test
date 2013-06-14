// histo.cpp


#include "histo.h"
#include <cstdlib>
#include <cmath>


CHistogram::CHistogram(int min, int max, int bin)
{
	m_data = NULL;
	if (min>=max || bin<1) return;
	m_min = min;
	m_max = max;
	m_bin = bin;
	n = (max - min)/bin;
	if (n>0) m_data = new unsigned int[n];
	Clear();
}


void CHistogram::Clear()
{
	m_total = m_integral = 0;
	if (m_data) for (int i=0; i<n; i++) m_data[i] = 0;
}


void CHistogram::AddData(int value)
{
	if (!m_data) return;
	m_total++;
	int i = (value-m_min)/m_bin;
	if (i>=0 && i<n)
	{
		m_integral++;
		m_data[i]++;
	}
}


unsigned int CHistogram::Peak()
{
	if (!m_data) return 0;

	unsigned int peak = 0;
	for (int i=0; i<n; i++) if(m_data[i] > peak) peak = m_data[i];
	return peak;
}

bool CHistogram::FindPeak(int &x, int &p)
{
	int left, right;
	for (left = x;    (left <n) && (m_data[left] <10); left++);
	for (right= left; (right<n) && (m_data[right]>10); right++);
	if (right > left) { p = (left+right)/2; x = right+10; return true; }
	return false;
}


void CHistogram::ScanPeaks(int data[], int size, int &count)
{
	int x = 0;
	for (count=0; count<size; count++)
	{
		if (!FindPeak(x, data[count])) break;
	}
	for (int i=0; i<count; i++) data[i] = data[i]*5 + m_min;
}


void CHistogram::Print(CProtocol &f, unsigned int linesize)
{
	if (!m_data) return;
	int i, x;
	f.printf("histogram (%u/%u)", m_total, m_integral);
	for (i=0, x=m_min; i<n; i++, x+=m_bin)
	{
		if (i%linesize == 0) f.printf("\n%6i", x);
		f.printf(" %5u", m_data[i]);
	}
	f.puts("\n");
}


void CHistogram::ScaleX(CProtocol &f)
{
	int p, i, x;
	int m = abs(m_max);
	if (abs(m_min)>m) m = abs(m_min);
	for (p=1; p<m; p*=10)
	{
		f.puts("      ");
		for (i=0,x=m_min; i<n; i++,x+=m_bin)
		{
			int k = (abs(x)/p)%10;
			f.printf("%i", k);
		}
		f.puts("\n");
	}
	f.puts("      ");
	for (i=0,x=m_min; i<n; i++,x+=m_bin)
		if (x<0) f.puts("-"); else f.puts("+");
	f.puts("\n");
}


void CHistogram::LogScaleY(int y, CProtocol &f)
{
	if (y%4 == 0) f.printf("\n10^%i +",y/4);
	else f.puts("\n     |");
}


void CHistogram::Plot(CProtocol &f, unsigned int scale)
{
	if (!m_data) return;

	f.printf("total    = %u\n", m_total);
	f.printf("integral = %u\n", m_integral);

	unsigned int peak = Peak();
	int y, i;
	for (y = scale; y >= 0; y--)
	{
		for (i=0; i<n; i++)
			if (m_data[i]*scale > y*peak) f.puts("*"); else f.puts(" ");
		f.puts("\n");
	}
	for (i=0; i<n; i++) f.puts("=");
	f.puts("\n");
	ScaleX(f);
}


void CHistogram::LogPlot(CProtocol &f)
{
	if (!m_data) return;

	f.printf("total    = %u\n", m_total);
	f.printf("integral = %u\n", m_integral);

//	unsigned int peak = Peak();
//	unsigned int maxy = 1;
//	while (maxy<peak) maxy*=10;
	const double k = log(1e7/0.7)/28;
//	double k = peak ? log((double)maxy/0.9)/21 : 0.0;
	int y, i;
	for (y = 28; y >= 0; y--)
	{
		LogScaleY(y,f);
		unsigned int ly = (unsigned int)(exp(k*y)*0.7);
		for (i=0; i<n; i++)
			if (m_data[i] > ly) f.puts("*"); else f.puts(" ");
	}
	f.puts("\n     |"); for (i=0; i<n; i++) f.puts("=");
	f.puts("\n");
	ScaleX(f);
}
