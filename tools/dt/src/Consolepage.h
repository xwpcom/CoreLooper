#pragma once

#include "basepage.h"
#include "loginfo.h"
class LogManager;
class LogPage;
#define WM_SINK_MSG	(WM_USER+1)
class MsgSinkWnd : public CWnd
{
public:
	MsgSinkWnd();
	BOOL Create(CString szName);
	virtual ~MsgSinkWnd();

	sigslot::signal2<LPBYTE,int> SignalCopyDataReady;

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL Register();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()
};

class ConsolePage : public BasePage,public sigslot::has_slots<>
{
	DECLARE_DYNAMIC(ConsolePage)

public:
	ConsolePage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~ConsolePage();
	void PreClose();
	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ConsolePage };
#endif
	sigslot::signal1<shared_ptr<LogItem>> SignalLogItemReady;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void UpdateControlPos();
	DECLARE_MESSAGE_MAP()

	int Init();
	void OnTimer(UINT_PTR nIDEvent);

	void OnCopyDataReady(LPBYTE data, int bytes);
	void OnContextMenu(CWnd* pWnd, CPoint point);
	
	void LoadConfig()override;
	void SaveConfig()override;

	CMFCTabCtrl	mTab;
	CTabCtrl	mTabPos;
	MsgSinkWnd	mSinkWnd;
	list<shared_ptr<BasePage>> mPages;
	shared_ptr<LogManager> mLogManager;

	shared_ptr<LogPage> CreateTab(string title, int id);
	int GetFreePageId();
	int mMaxLogCount = 1000 * 1000;
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTabDelete();
	afx_msg void OnTabEdit();
	afx_msg void OnTabNew();
};
