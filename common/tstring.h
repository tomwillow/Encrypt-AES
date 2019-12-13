#pragma once
#include <windows.h>
#include "tchar_head.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <string>

using std::endl;

#ifdef _UNICODE

using std::wstring;
#define tstring wstring

using std::wostream;
#define tostream wostream

using std::wifstream;
#define tifstream wifstream

using std::wofstream;
#define tofstream wofstream

using std::wistringstream;
#define tistringstream wistringstream

using std::wostringstream;
#define tostringstream wostringstream

using std::wcout;
#define tcout wcout

using std::to_wstring;
#define tto_string to_wstring

#else

using std::string;
#define tstring string

using std::ostream;
#define tostream ostream

using std::ifstream;
#define tifstream ifstream

using std::ofstream;
#define tofstream ofstream

using std::istringstream;
#define tistringstream istringstream

using std::ostringstream;
#define tostringstream ostringstream

using std::cout;
#define tcout cout

using std::to_string;
#define tto_string to_string

#endif

inline tstring & operator<<(tstring &s, double d)
{
	TCHAR temp[32];
	_stprintf_s(temp, TEXT("%f"), d);
	s += temp;
	return s;
}

inline tstring & operator<<(tstring &s, size_t i)
{
	TCHAR temp[32];
#ifdef _UNICODE
	_itow_s(i, temp, 10);
#else
	_itoa_s(i, temp, 10);
#endif
	s += temp;
	return s;
}

inline tstring & operator<<(tstring &s, int i)
{
	TCHAR temp[32];
#ifdef _UNICODE
	_itow_s(i, temp, 10);
#else
	_itoa_s(i, temp, 10);
#endif
	s += temp;
	return s;
}

inline tstring & operator<<(tstring &s, TCHAR szStr[])
{
	s += szStr;
	return s;
}

inline tstring & operator<<(tstring &s,const tstring &s2)
{
	s += s2;
	return s;
}

std::wstring stringToWstring(const std::string& str);
std::string wstringToString(const std::wstring& wstr);