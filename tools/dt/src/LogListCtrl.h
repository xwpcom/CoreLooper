#pragma once

class ListCtrlNodeProxy
{
public:
	virtual ~ListCtrlNodeProxy() {}

	virtual COLORREF OnGetCellTextColor(DWORD_PTR context,int nRow, int nColum)=0;
	virtual COLORREF OnGetCellBkColor(DWORD_PTR context, int nRow, int nColum)=0;
};

class LogListCtrl : public CMFCListCtrl
{
	DECLARE_DYNAMIC(LogListCtrl)

public:
	LogListCtrl();
	virtual ~LogListCtrl();
	sigslot::signal2<CWnd*,CPoint> SignalContextMenu;

	void SetNodeProxy(shared_ptr<ListCtrlNodeProxy> obj)
	{
		mNodeProxy = obj;
	}

protected:
	DECLARE_MESSAGE_MAP()

	virtual COLORREF OnGetCellTextColor(int nRow, int nColum);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColum);

	shared_ptr<ListCtrlNodeProxy> mNodeProxy;
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


