// perftimer.h

#ifndef PERFTIMER_H
#define PERFTIMER_H

#include <windows.h>
#include <intrin.h>


class CPerformanceTimer
{
	long long f;
	long long total;
	long long start;
	unsigned long count;
/*	long long rdtsc()
	{
		long long t;
		__asm
		{
			rdtsc
			mov DWORD PTR t,eax
			mov DWORD PTR t+4,edx
		}
		return t;
	} */
public:
	CPerformanceTimer() : total(0), count(0)
	{
		if (!QueryPerformanceFrequency(PLARGE_INTEGER(&f)))
			throw "QueryPerformanceFrequency";
	};
	static void SetSingleCore()
	{
		if (SetThreadAffinityMask(GetCurrentThread(), 1) == 0)
			throw "SetThreadAffinityMask";
	};
	void Reset() { total = 0; count = 0; }
	void Start() { count++; start = __rdtsc(); }
	void Stop()  { total += __rdtsc() - start; }
	double Elapsed() { return double(total)/f; }
	unsigned long NumCalls() { return count; }
};


class CWinPerformanceTimer
{
public:
	long long f;
	long long total;
	long long start;
	unsigned long count;
public:
	CWinPerformanceTimer() : total(0), count(0)
	{
		if (!QueryPerformanceFrequency(PLARGE_INTEGER(&f)))
			throw "Elapsed.QueryPerformanceFrequency";
	};
	void Reset() { total = 0; count = 0; }
	void Start()
	{
		count++;
		if (!QueryPerformanceCounter(PLARGE_INTEGER(&start)))
			throw "Start.QueryPerformanceTimer";
	}
	void Stop()
	{
		long long stop;
		if (!QueryPerformanceCounter(PLARGE_INTEGER(&stop)))
			throw "Stop.QueryPerformanceTimer";
		total += stop - start;
	}
	double Elapsed() { return (double(total)/f)*1000.0;	}
	unsigned long NumCalls() { return count; }
};


#endif
