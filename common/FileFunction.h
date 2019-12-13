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
		//lpstrFilter��ʽ��TEXT("��������ļ�(*.lml)\0*.lml\0\0")
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
		ofn.lpstrFilter = szFilter;//����\0��ʾ����
	}

	void SetTitle(tstring title)
	{
		_tcscpy_s(szTitle, title.length() + 1, title.c_str());
		ofn.lpstrFileTitle = szTitle;
	}

	bool Open(tstring& fileName)
	{
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;//�޶��ļ��������
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

		//��Ϊ�տ����Զ�����ѡ��ĺ�׺������������ѡʲô��׺��
		//ֻҪû������.txt���֣������޺�׺
		ofn.lpstrDefExt = TEXT(""); 
		if (::GetSaveFileName(&ofn))
		{
			fileName = ofn.lpstrFile;
			return true;
		}
		return false;
	}
};

//����index=1��õ������ļ���
//�ɹ�ȡ�÷���true 
bool GetCommandLineByIndex(int index, TCHAR* assigned);

//�ж��ļ��Ƿ����
bool GetFileExists(const TCHAR filename[]);

std::vector<std::string> SplitPath(const std::string& s);

std::vector<std::string> SplitFileName(const std::string& s);
