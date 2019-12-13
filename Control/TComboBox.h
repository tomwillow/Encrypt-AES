#pragma once
#include "TControl.h"
#include <unordered_map>

class TComboBox :
	public TControl
{
private:
	std::vector<int> items;
public:
	void AddItem(const tstring s, int value);

	int GetCurSelData();

	void SetCurSel(int i);
};

