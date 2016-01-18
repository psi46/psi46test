/* -------------------------------------------------------------
 *
 *  file:        test.h
 *
 *  description: test procedures for PSI46V2
 *
 *  author:      Beat Meier
 *  modified:    24.1.2004
 *
 *  rev:
 *
 * -------------------------------------------------------------
 */

#ifndef TEST_H
#define TEST_H

#include "config.h"

namespace TestPROC600
{

void InitDAC(bool reset = false);
void InitChip();

void WriteSettings();
void test_current();
int test_tout();
int test_i2c();
int test_TempSensor();
int getTemperature();
bool test_Pixel();

int test_roc(bool &repeat);
int test_roc_bumpbonder();

}


namespace TestRocDig
{

void InitDAC(bool reset = false);
void InitChip();

void WriteSettings();
void test_current();
int test_tout();
int test_i2c();
int test_TempSensor();
int getTemperature();
bool test_Pixel();

int test_roc(bool &repeat);
int test_roc_bumpbonder();

}


namespace TestRocAna
{

void InitDAC(bool reset = false);
void InitChip();

void WriteSettings();
void test_current();
int test_tout();
int test_i2c();
int test_TempSensor();
int getTemperature();
bool test_Pixel();

int test_roc(bool &repeat);
int test_roc_bumpbonder();

}

#endif
