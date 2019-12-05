#pragma once
#include "TEdit.h"
#include <vector>
#include <string>

class TDropEdit :public TEdit
{
protected:
	void DropProc(const std::vector<std::string>& dropFiles)override
	{
		if (dropFiles.size() == 1)
			this->SetText(dropFiles[0]);
	}
};