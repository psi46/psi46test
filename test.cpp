// test.cpp

#include "psi46test.h"


CChip g_chipdata;


int test_roc(bool &repeat)
{
	switch (settings.rocType)
	{
		case 0:  return TestRocAna::test_roc(repeat);
		case 1:  return TestRocDig::test_roc(repeat);
		default: return TestPROC600::test_roc(repeat);
	}
}


int test_roc_bumpbonder()
{
	switch (settings.rocType)
	{
		case 0:  return TestRocAna::test_roc_bumpbonder();
		case 1:  return TestRocDig::test_roc_bumpbonder();
		default: return TestPROC600::test_roc_bumpbonder();
	}
}
