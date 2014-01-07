// plot.h

#pragma once

#include "CImg.h"
using namespace cimg_library;


void Scope(const char *title, std::vector<double> &values);

void PlotData(const char *title, const char *xaxis, const char *yaxis,
	double xmin, double xmax, std::vector<double> &values);

class CDotPlot
{
	static const int x0, y0;
	CImg<unsigned char> img;
public:
	CDotPlot();
	~CDotPlot();
	void Add(int x, int y, int value);
	void AddMean(int x, double y);
	void Show();
};
