#pragma once
#include "TEdit.h"
#include <vector>
#include <string>

class TDropEdit :public TEdit
{
protected:
	LRESULT DropProc(const std::vector<std::tstring>& dropFiles)override
	{
		if (dropFiles.size() == 1)
			this->SetText(dropFiles[0]);
		return TRUE;
	}
};