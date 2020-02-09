#include "pch.h"
#include "LogListCtrl.h"

IMPLEMENT_DYNAMIC(LogListCtrl, CMFCListCtrl)

LogListCtrl::LogListCtrl()
{
}

LogListCtrl::~LogListCtrl()
{
}

BEGIN_MESSAGE_MAP(LogListCtrl, CMFCListCtrl)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

COLORREF LogListCtrl::OnGetCellTextColor(int row, int col)
{
	if (mNodeProxy)
	{
		auto data = GetItemData(row);
		return mNodeProxy->OnGetCellTextColor(data, row, col);
	}

	return(row % 2) == 0 ? RGB(128, 37, 0) : RGB(0, 0, 0);
}

COLORREF LogListCtrl::OnGetCellBkColor(int row, int col)
{
	if (mNodeProxy)
	{
		auto data = GetItemData(row);
		return mNodeProxy->OnGetCellBkColor(data, row, col);
	}

	if (m_bMarkSortedColumn && col == m_iSortedColumn)
	{
		return(row % 2) == 0 ? RGB(233, 221, 229) : RGB(176, 218, 234);
	}

	return(row % 2) == 0 ? RGB(253, 241, 249) : RGB(196, 238, 254);
}


void LogListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	SignalContextMenu(this, point);
}
