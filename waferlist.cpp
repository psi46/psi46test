// waferlist.cpp


#include <stdio.h>
#include <time.h>
#include "waferlist.h"


bool CWaferId::Read(FILE *f)
{
	int i, p1, p2;
	char s[256];

	do
	{
		if (!fgets(s, 255, f)) return feof(f) != 0;
		
		for (i = 0; s[i]; i++) if (s[i] < ' ') s[i] = ' ';

		// find begin of first string -> p1
		p1 = 0;
		while (s[p1] == ' ') p1++;
	} while (s[p1] == 0);

	// find end of first string -> p2
	p2 = p1;
	while (s[p2] > ' ') p2++;
	if ((p2-p1) <= 0) return false;

	id.assign(s, p1, p2-p1);

	// find begin of second string -> p2
	while (s[p2] == ' ') p2++;

	// find end of second string -> p1
	p1 = p2;
	while ((s[p1] != ' ') && (s[p1] != 0)) p1++;
	tested = (p1-p2) > 0;
	if (tested) timestamp.assign(s, p2, p1-p2);

	return true;
}


void CWaferId::Write(FILE *f)
{
	if (tested)
		fprintf(f, "%s %s\n", id.c_str(), timestamp.c_str());
	else
		fprintf(f, "%s\n", id.c_str());
}


void CWaferId::SetTested()
{
	time_t t_raw;
	tm *t;

	time(&t_raw);
	t = localtime(&t_raw);

	tested = true;
	char s[64];
	sprintf(s, "%04i%02i%02i%02i%02i",
		t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
	timestamp = s;
}


bool CWaferList::Read(const std::string &fileName)
{
	FILE *f = fopen(fileName.c_str(), "rt");
	if (!f) return false;
	while (!feof(f))
	{
		CWaferId w;
		if (!w.Read(f)) { wafer.clear(); fclose(f); return false; }
		if (!feof(f)) wafer.push_back(w);
	}
	fclose(f);
	modified = false;
	return true;
}


void CWaferList::Write(const std::string &fileName)
{
	if (!modified) return;

	FILE *f = fopen(fileName.c_str(), "wt");
	if (!f) return;
	std::list<CWaferId>::iterator i;
	for (i = wafer.begin(); i != wafer.end(); i++) i->Write(f);
}


int CWaferList::SelectWafer(const std::string &waferId)
{
	for (current = wafer.begin(); current != wafer.end(); current++)
	{
		if (current->IsId(waferId))
		{
			if (!current->IsTested()) return 0; // ok
			else
			{
				current = wafer.end();
				return 2; // already tested
			}
		};
	}
	return 1;  // not existing
}


void CWaferList::SetTested()
{
	if (current != wafer.end())
	{
		current->SetTested();
		modified = true;
	}
}


const std::string defaultName = "?";

const std::string& CWaferList::GetId()
{
	if (current != wafer.end()) return current->GetId();
	return defaultName;
}
