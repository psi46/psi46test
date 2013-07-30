// profiler.h

#pragma once

#include <string>
#include <list>

// MSVC: __FUNCTION__ __FUNCDNAME__ __FUNCSIG__
// GCC:  __func__     __FUNCTION__  __PRETT_FUNCTION__

#define PROFILING // profiler disabled
// #define PROFILING static Watchpoint profiler_watchpoint(__FUNCTION__); AutoCounter profiler_counter(profiler_watchpoint);


#ifdef _WIN32

class Watchpoint
{
	static long long frequency;
	static std::list<Watchpoint*> *wplist;

	std::string name;
	unsigned int n;
	long long t;

	bool IsRunning() { return wplist != 0; }
	void Incr(long long dt) { t += dt; n++; }
	static void Report(const char *filename);
public:
	Watchpoint(const char *fname);
	~Watchpoint();
	friend class AutoCounter;
};



class AutoCounter
{
	Watchpoint *m_wp;
	long long m_start;
	void Start() { m_start = __rdtsc(); }
	void Stop() { m_wp->Incr(__rdtsc() - m_start); }
public:
	AutoCounter(Watchpoint &handle) : m_wp(&handle) { Start(); }
	~AutoCounter() { Stop(); }
};

#endif
