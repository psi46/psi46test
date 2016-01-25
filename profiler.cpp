
// profiler.cpp

#ifdef _WIN32

#include <windows.h>
#include <intrin.h>
#include "profiler.h"


long long Watchpoint::frequency = 0;
std::list<Watchpoint*> Watchpoint::wplist;


Watchpoint::Watchpoint(const char *fname) : name(fname), t(0), n(0)
{
	if (!IsRunning())
	{
		if (SetThreadAffinityMask(GetCurrentThread(), 1) == 0) // Linux: sched_setaffinity
		{
			printf("ERROR profiler\n");
			exit(1);
		}

		if (!QueryPerformanceFrequency(PLARGE_INTEGER(&frequency)))
		{
			printf("ERROR profiler\n");
			exit(2);
		}
	}

	wplist.push_back(this);
}


Watchpoint::~Watchpoint()
{
	if (IsRunning())
	{
		Report("profiler_report.txt");
		wplist.clear();
	}
}


bool wplist_sorter(Watchpoint *a, Watchpoint *b) { return *b < *a; }

void Watchpoint::Report(const char *filename)
{
	FILE *f = fopen(filename, "wt");
	if (!f) return;

	wplist.sort(wplist_sorter);

	std::list<Watchpoint*>::iterator i;

	unsigned int width = 0;
	for (i = wplist.begin(); i != wplist.end(); i++)
		if ((*i)->name.size() > width) width = (*i)->name.size();
	if (width > 300) width = 300;

	// --- print title
	fprintf(f, "%-*s %s\n", width+1, "Function", "Counts    Total ms");
	for (int k = 0; k < width+20; k++) fputc('-', f);
	fprintf(f, "\n");

	// --- print list
	for (i = wplist.begin(); i != wplist.end(); i++)
	{
		fprintf(f, "%-*s %7i %11.3f\n", width,
			(*i)->name.c_str(), (*i)->n, double((*i)->t)/frequency);
	}
}

#endif

