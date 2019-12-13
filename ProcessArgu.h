#pragma once

#include <Windows.h>
#include "TProgress.h"
#include "TCheckBox.h"
#include "TDropEdit.h"
#include "TButton.h"
#include <vector>
#include "ScopeTime.h"

class ProcessArgu
{
private:
	const TCHAR* AppTitle;
	HWND hDlg;
	TProgress* pProgress;
	TButton* pBtnConvert;
	tstring sBtnConvert;
	std::vector<TControl*> vechShouldDisable;
public:
	ScopeTime st;
	bool encrypt;
	tstring fileName1, fileName2;
	tstring key;
	tstring iv;
	bool quick;
	FILE* fpp1, * fpp2;
	bool success;
	long dealBytes;
	ProcessArgu() = delete;
	ProcessArgu(const ProcessArgu&) = delete;
	ProcessArgu(const TCHAR AppTitle[],
		bool encrypt,
		tstring fileName1, tstring fileName2,
		tstring key, tstring iv,bool quick,
		HWND hDlg, TProgress* pProgress,
		 TButton* pBtnConvert,
		const std::initializer_list<TControl*>& vechShouldDisable) :
		AppTitle(AppTitle),
		encrypt(encrypt), fileName1(fileName1), fileName2(fileName2),
		key(key),iv(iv),quick(quick),
		hDlg(hDlg), pProgress(pProgress),
		pBtnConvert(pBtnConvert),
		vechShouldDisable(vechShouldDisable)
	{
		fpp1 = nullptr;
		fpp2 = nullptr;
		success = false;

		//全局控件禁用
		for (auto h : vechShouldDisable)
			h->SetEnable(false);

		sBtnConvert = pBtnConvert->GetText();

		pProgress->SetPos(0);
	}

	~ProcessArgu()
	{
		//释放资源
		if (fpp1)
			fclose(fpp1);
		if (fpp2)
			fclose(fpp2);

		//恢复控件状态
		for (auto h : vechShouldDisable)
			h->SetEnable(true);

		pBtnConvert->SetText(sBtnConvert);

		//设置完成状态并弹窗
		if (success)
		{
			double velocityKBperS = (dealBytes) / 1024.0 / (st.elapsedMilliseconds() / 1000.0);
			TCHAR caption[MAX_PATH];
			_stprintf_s(caption,MAX_PATH, TEXT("处理完成。\n时间：%s\n速度：%.0f KB/s"), st.elapsed().c_str(), velocityKBperS);
			pProgress->SetPos(100);
			MessageBox(hDlg, caption, TEXT(""), MB_OK | MB_ICONINFORMATION);

			SetWindowText(hDlg, AppTitle);
		}
	}

	//更新进度条
	void SetProgress(int per, double velocityKBperS, double remainSecond)
	{
		pProgress->SetPos(per);
		tstring sPercent = tto_string(per) + TCHAR("%");
		pBtnConvert->SetText(sPercent);

		TCHAR title[MAX_PATH];
		_stprintf_s(title, MAX_PATH, TEXT("%s 速度:%.0f KB/s 当前:%d%% 预计剩余:%.2f s"), AppTitle, velocityKBperS, per, remainSecond);
		SetWindowText(hDlg, title);
	};
	friend void DealProc(void* p);
};

void DealProc(void* p);