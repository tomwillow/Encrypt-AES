#pragma once

#include <Windows.h>
#include <vector>
#include <tchar.h>

#include "TProgress.h"
#include "TCheckBox.h"
#include "TDropEdit.h"
#include "TButton.h"

#include "ScopeTime.h"

class ProcessArgu
{
private:
	ScopeTime st;
	bool encrypt;

	//位宽，分组模式，查表加速
	int bits, mode;
	bool quick;

	//key, iv
	std::string key,iv;

	//输入输出文件名
	std::tstring fileName1, fileName2;

	//完成标记以及处理字节数
	bool success;
	long long dealBytes;

	const TCHAR* AppTitle;//原app标题，用于在设置窗口标题进度后恢复
	HWND hDlg;//用于弹出模态对话框
	TProgress* pProgress;//进度条控件，用于设置进度
	TButton* pBtnConvert;//传入按钮用于设置进度百分比
	std::tstring BtnConvertText;//暂存按钮文字用于从百分比中恢复
	std::vector<TControl*> vechShouldDisable;//存入各控件指针，用于处理中途禁用控件
public:
	ProcessArgu() = delete;
	ProcessArgu(const ProcessArgu&) = delete;
	ProcessArgu(const TCHAR AppTitle[],
		bool encrypt,
		int bits,int mode,bool quick,
		std::string key, std::string iv,
		std::tstring fileName1, std::tstring fileName2,
		HWND hDlg, TProgress* pProgress,
		TButton *pBtnConvert,
		const std::initializer_list<TControl*>& vechShouldDisable) :

		AppTitle(AppTitle),
		encrypt(encrypt), 
		bits(bits),mode(mode),quick(quick),
		key(key),iv(iv),
		fileName1(fileName1), fileName2(fileName2),
		hDlg(hDlg), pProgress(pProgress),
		pBtnConvert(pBtnConvert),
		vechShouldDisable(vechShouldDisable)
	{
		success = false;

		//全局控件禁用
		for (auto h : vechShouldDisable)
			h->SetEnable(false);

		//暂存加/解密按钮文字
		BtnConvertText = pBtnConvert->GetText();

		//初始化进度条
		pProgress->SetPos(0);
	}

	~ProcessArgu()
	{
		//恢复控件状态
		for (auto h : vechShouldDisable)
			h->SetEnable(true);

		//恢复加/解密按钮文字
		pBtnConvert->SetText(BtnConvertText);

		//如果已设置完成状态，则弹窗
		if (success)
		{
			//计算速度
			double velocityKBperS = (dealBytes) / 1024.0 / (st.elapsedMilliseconds() / 1000.0);

			//设置对话框内容
			TCHAR caption[MAX_PATH];
			_stprintf_s(caption,MAX_PATH, TEXT("处理完成。\n时间：%s\n速度：%.0f KB/s"), st.elapsed().c_str(), velocityKBperS);
			
			//设置进度条
			pProgress->SetPos(100);

			//弹框
			MessageBox(hDlg, caption, TEXT(""), MB_OK | MB_ICONINFORMATION);

			//恢复app标题
			SetWindowText(hDlg, AppTitle);
		}
	}

	//更新进度条
	void SetProgress(int per, double velocityKBperS, double remainSecond);

	//处理函数
	friend void DealProc(void* p);
};

void DealProc(void* p);