
// profiler.cpp

#ifdef _WIN32

#include <windows.h>
#include <intrin.h>
#include "profiler.h"


long long Watchpoint::frequency = 0;
std::list<Watchpoint*> *Watchpoint::wplist = 0;


Watchpoint::Watchpoint(const char *fname)
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

		wplist = new std::list<Watchpoint*>;
	}

	name = fname;
	t = 0;
	n = 0;
	wplist->push_back(this);
}


Watchpoint::~Watchpoint()
{
	if (!IsRunning()) return;

	Report("profiler_report.txt");
	delete wplist;
	wplist = 0;
}


void Watchpoint::Report(const char *filename)
{
	FILE *f = fopen(filename, "wt");
	if (!f) return;

	std::list<Watchpoint*>::iterator i;

	unsigned int width = 0;
	for (i = wplist->begin(); i != wplist->end(); i++)
		if ((*i)->name.size() > width) width = (*i)->name.size();
	if (width > 300) width = 200;
	for (i = wplist->begin(); i != wplist->end(); i++)
	{
		fprintf(f, "%-*s %7i %11.3f\n", width,
			(*i)->name.c_str(), (*i)->n, double((*i)->t)/frequency);
	}
}

#endif

