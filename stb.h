// stb.h

#pragma once

namespace stb {

void Sectortest(const char *name);

bool StartModules();
void StopModules();

void HvTest();
void RdaTest(int sel = -1, int hub = -1);
void AssignHub(unsigned int sel, int hub = -1);
void DelayScan(unsigned int sel, unsigned int group);

}
