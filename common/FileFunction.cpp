#include "FileFunction.h"
#include <tchar.h>

using namespace std;

void TFileDialog::SetFilter(std::vector<std::pair<std::string, std::string>> vecFilter)
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

void TFileDialog::SetTitle(std::tstring title)
{
	_tcscpy_s(szTitle, title.length() + 1, title.c_str());
	ofn.lpstrFileTitle = szTitle;
}

bool TFileDialog::Open(std::tstring& fileName)
{
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;//限定文件必须存在
	if (::GetOpenFileName(&ofn))
	{
		fileName = ofn.lpstrFile;
		return true;
	}
	return false;
}

void TFileDialog::SetszFile(const std::tstring& s)
{
	_tcscpy_s(szFile, s.c_str());
}

std::tstring TFileDialog::GetszFile()
{
	return std::tstring(szFile);
}

bool TFileDialog::Save(std::tstring& fileName)
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

//传入index=1则得到传入文件名
//成功取得返回true 
bool GetCommandLineByIndex(int index, TCHAR *assigned)
{
	int iCmdLineCount = -1;
	size_t len = _tcslen(GetCommandLine());
	TCHAR *origin = new TCHAR[len + 1];
	TCHAR *s = origin;
	_tcscpy_s(s,len, GetCommandLine());
	bool inchar = false;
	TCHAR *start=nullptr, *end=nullptr;
	while ((s = _tcschr(s, TEXT('\"'))) != NULL)
	{
		s++;
		if (inchar == false)//开端
		{
			start = s;
			inchar = true;
		}
		else
		{
			end = s - 1;
			iCmdLineCount++;
			if (iCmdLineCount == index)
			{
				_tcsncpy_s(assigned,end-start, start, end - start);
				assigned[end - start] = TEXT('\0');
				break;
			}
			inchar = false;
		}
	}
	delete[] origin;
	if (iCmdLineCount == index)
		return true;
	else
		return false;
}


bool GetFileExists(const TCHAR filename[])
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(filename, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}
	else {
		FindClose(hFind);
		return true;
	}
}

//路径，文件名不带后缀，后缀
vector<string> SplitPath(const std::string& s)
{
	vector<string> ret;
	auto slash = s.find_last_of("/\\");
	if (slash != string::npos)
	{
		ret.push_back(s.substr(0, slash));
		auto vec = SplitFileName(s.substr(slash+1));
		ret.insert(ret.end(), vec.begin(),vec.end());
	}
	else
	{
		ret.push_back("");
		auto vec=SplitFileName(s);
		ret.insert(ret.end(), vec.begin(), vec.end());
	}
	return ret;
}

//文件名不带后缀，后缀
vector<string> SplitFileName(const std::string& s)
{
	auto dot = s.find_last_of('.');
	if (dot != string::npos)
		return { s.substr(0,dot),s.substr(dot) };
	else
		return { s,"" };
}