// waferlist.h

#ifndef WAFERLIST_H
#define WAFERLIST_H

#include <string>
#include <list>


class CWaferId
{
	std::string id;
	bool tested;
	std::string timestamp;  // JJJJMMDDhhmm
public:
	bool Read(FILE *f);
	void Write(FILE *f);
	void SetTested();
	bool IsId(const std::string &waferId) { return id == waferId; }
	bool IsTested() { return tested; }
	const std::string& GetId() { return id; }
};

class CWaferList
{
	bool modified;
	std::list<CWaferId>::iterator current;
public:
	CWaferList() : modified(false), current(wafer.end()) {}
	bool Read(const std::string &fileName);
	void Write(const std::string &fileName);
	int SelectWafer(const std::string &waferId); // 0 = ok, 1 = not existing, 2 = already tested
	void SetTested();
	const std::string& GetId();

	std::list<CWaferId> wafer;
};


#endif
