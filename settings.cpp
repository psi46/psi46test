// settings.cpp

#include "settings.h"


bool CSettings::read_int(int &value, int min, int max)
{
	char s[200];
	if(fgets(s, 198, f) == NULL) return false;
	if (s == NULL) return false;

	int v;
	if (sscanf(s,"%i",&v) != 1) return false;
	if (v<min || max<v) return false;
	value = v;

	return true;
}


bool CSettings::read_string(char string[], int size)
{
	char s[256];
	if(fgets(s, 254, f) == NULL) return false;
	if (s == NULL) return false;

	int i = 0;
	int n = size-1;
	while (s[i] != 0 && s[i] != ' ' && i < n)
	{
		string[i] = s[i];
		i++;
	}
	s[i] = 0;
	return true;
}


bool CSettings::read(const char filename[])
{
	bool ok = false;
	f = fopen(filename, "rt");
	if (!f) return false;

	if (!read_string(port_tb,     20     )) goto read_error;
	if (!read_string(path,       254     )) goto read_error;
	if (!read_int(port_prober,    -1,  99)) goto read_error;
	if (!read_int(sensor,          0,   1)) goto read_error;
	if (!read_int(clock,           0,  50)) goto read_error;
	if (!read_int(errorRep,        0,   5)) goto read_error;
	if (!read_int(l1_bl_shift,  -100, 100)) goto read_error;

	if (!read_int(vcomp,           0, 255)) goto read_error;
	if (!read_int(vhlddel,         0, 255)) goto read_error;
	if (!read_int(vthr,            0, 255)) goto read_error;
	if (!read_int(caldel,          0, 255)) goto read_error;
	if (!read_int(vcal,            0, 255)) goto read_error;

	ok = true;
read_error:
	fclose(f);
	return ok;
}
