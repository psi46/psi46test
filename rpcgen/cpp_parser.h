// cpp_parser.h

#pragma once

#include <string>
using namespace std;


class CppParser
{
	bool empty_fnct;
	bool typeUnsigned; // f = signed, t = unsigned
	char typeChar;   // 'b' = bool, 'c' = char, 's' = short, 'i' = int, 'l' = long, 'd' = long long
	char ctypeChar;  // 0 = simple, '0' = ref, '1' = vector, '2' = vectorR, '3' = String, '4' = StringR, '5' = HWvector
	string fname;
	string fparam;

	CppScanner f;

	void GetIntegerType();
	void GetType();
	void GetComplexType();
	void GetParameter();
	void GetParList();
	void GetModifier();
	void GetFunction();
public:
	CppParser() {}
	void Open(const char *fileName) { f.Open(fileName); }
	void Close() { f.Close(); }

	void GetFunctionDecl();
	unsigned char GetRpcExport();

	bool IsEmptyFnct() { return empty_fnct; }
	const string& GetFname() { return fname; }
	const string& GetFparam() { return fparam; }
};
