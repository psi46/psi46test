// plot.h

#pragma once

void Scope(const char *title, std::vector<double> &values);

void PlotData(const char *title, const char *xaxis, const char *yaxis,
	double xmin, double xmax, std::vector<double> &values);


