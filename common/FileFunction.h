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

	void SetFilter(std::vector<std::pair<std::string, std::string>> vecFilter)
	{
		//lpstrFilter格式：TEXT("机构设计文件(*.lml)\0*.lml\0\0")
		int i = 0;
		for (auto& pr : vecFilter)
		{
			for (auto c : pr.first)
				szFilter[i++] = c;
			szFilter[i++] = 0;
			for (auto c : pr.second)
				szFilter[i++] = c;
			szFilter[i++] = 0;
		}
		szFilter[i++] = 0;
		ofn.lpstrFilter = szFilter;//两个\0表示结束
	}

	void SetTitle(tstring title)
	{
		_tcscpy_s(szTitle, title.length() + 1, title.c_str());
		ofn.lpstrFileTitle = szTitle;
	}

	bool Open(tstring& fileName)
	{
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;//限定文件必须存在
		if (::GetOpenFileName(&ofn))
		{
			fileName = ofn.lpstrFile;
			return true;
		}
		return false;
	}

	void SetszFile(const tstring &s)
	{
		_tcscpy_s(szFile, s.c_str());
	}

	tstring GetszFile()
	{
		return tstring(szFile);
	}

	bool Save(tstring& fileName)
	{
		ofn.Flags = OFN_PATHMUSTEXIST;

		//设为空可以自动加上选择的后缀名，否则无论选什么后缀，
		//只要没有输入.txt这种，都是无后缀
		ofn.lpstrDefExt = TEXT(""); 
		if (::GetSaveFileName(&ofn))
		{
			fileName = ofn.lpstrFile;
			return true;
		}
		return false;
	}
};

//传入index=1则得到传入文件名
//成功取得返回true 
bool GetCommandLineByIndex(int index, TCHAR* assigned);

//判断文件是否存在
bool GetFileExists(const TCHAR filename[]);

std::vector<std::string> SplitPath(const std::string& s);

std::vector<std::string> SplitFileName(const std::string& s);
