#pragma once
//XiongWanPing 2016.10.12
enum
{
	WM_TAB_SWAPED	=	(WM_USER+300),//
	WM_TAB_CHANGE_ACTIVE_TAB,
//	WM_TAB_CONTEXT_MENU,
};
class WIN_CLASS CMFCTabCtrlEx :public CMFCTabCtrl
{
public:
	void SwapTabs(int nFisrtTabID, int nSecondTabID)
	{
		//DV("SwapTabs(%d,%d)", nFisrtTabID, nSecondTabID);
		__super::SwapTabs(nFisrtTabID, nSecondTabID);

		GetParent()->PostMessage(WM_TAB_SWAPED, (WPARAM)GetSafeHwnd(),MAKELPARAM(nFisrtTabID, nSecondTabID));
	}

	virtual void OnChangeTabs()
	{
		//DW("%s", __func__);
		__super::OnChangeTabs();
	}

	virtual void FireChangeActiveTab(int nNewTab)
	{
		//DW("%s,nNewTab=%d", __func__, nNewTab);
		GetParent()->PostMessage(WM_TAB_CHANGE_ACTIVE_TAB, (WPARAM)GetSafeHwnd(), (LPARAM)nNewTab);
		__super::FireChangeActiveTab(nNewTab);
	}

	DECLARE_MESSAGE_MAP()
	//afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};
