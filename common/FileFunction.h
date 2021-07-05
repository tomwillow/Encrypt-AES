#pragma once

#include "tstring.h"
#include <vector>
#include <Windows.h>

class TFileDialog
{
private:
	OPENFILENAME ofn;
	TCHAR szTitle[MAX_PATH];
	TCHAR szFile[MAX_PATH];
	TCHAR szFilter[MAX_PATH];
	std::string initialDir;
public:
	TFileDialog() = delete;
	TFileDialog(const TFileDialog&) = delete;
	TFileDialog(HWND hwndOwner)
	{
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hwndOwner;
		szFile[0] = 0;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = NULL;
	}

	TFileDialog(HWND hwndOwner, std::vector<std::pair<std::string, std::string>> vecFilter):TFileDialog(hwndOwner)
	{
		SetFilter(vecFilter);
	}

	void SetFilter(std::vector<std::pair<std::string, std::string>> vecFilter);

	void SetTitle(std::tstring title);

	bool Open(std::tstring& fileName);

	void SetszFile(const std::tstring& s);

	std::tstring GetszFile();

	bool Save(std::tstring& fileName);
};

//传入index=1则得到传入文件名
//成功取得返回true 
bool GetCommandLineByIndex(int index, TCHAR* assigned);

//判断文件是否存在
bool GetFileExists(const TCHAR filename[]);

std::vector<std::string> SplitPath(const std::string& s);

std::vector<std::string> SplitFileName(const std::string& s);
