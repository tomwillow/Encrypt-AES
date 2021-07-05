#include "TProgress.h"
#include <commctrl.h>

void TProgress::SetPos(int pos)
{
	SendMessage(m_hWnd, PBM_SETPOS, pos, 0);
}