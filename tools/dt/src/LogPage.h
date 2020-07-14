#pragma once

#include "basepage.h"
#include "loginfo.h"
#include "LogListCtrl.h"
class LogFilterPage;
class LogItemPage;
class LogManager;
class LogPage : public BasePage, public sigslot::has_slots<>
{
	DECLARE_DYNAMIC(LogPage)

public:
	LogPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~LogPage();
	void SetMaxLogCount(int n)
	{
		mMaxLogCount = n;
	}
	void SetPageId(int id)
	{
		mPageId = id;
	}
	
	int pageId()const
	{
		return mPageId;
	}

	void SetLogManager(shared_ptr<LogManager> obj)
	{
		mLogManager = obj;
	}

	void OnLogItemReady(shared_ptr<LogItem> item);
	void RemoveSection();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LogPage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void OnRelayout(const CRect& rc)override;
	void OnClearLog();
	void OnFilterChanged();
	void AddItem(shared_ptr<LogItem>& item);
	void OnContextMenu(CWnd*, CPoint);
	void ClearLogListCtrl();

	int Init();
	CFont mFont;
	int mArrIdx[10];
	int mPageId = 0;

	shared_ptr<LogFilterPage> mFilterPage;
	shared_ptr<LogItemPage> mItemPage;
	list<shared_ptr<LogItem>> mLogItems;

	shared_ptr<LogManager> mLogManager;

	LogItem* GetCurrentLogItem();
	void dump(string desc);

	void LoadConfig()override;
	void SaveConfig()override;

	int mMaxLogCount = 1000 * 1000;
public:
	LogListCtrl mListCtrl;
	afx_msg void OnDestroy();
	afx_msg void OnNMRClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenFileGotoLine();
	void OnCodePageUtf8();
	void OnCodePageChinese();
	void OnUpdateCodePageUtf8(CCmdUI* pCmdUI);
	void OnUpdateCodePageChinese(CCmdUI* pCmdUI);

	afx_msg void OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCopyFullPath();
	afx_msg void OnOpenFolder();
	afx_msg void OnAppDisable();
	afx_msg void OnAppClear();
	afx_msg void OnTagDisable();
	afx_msg void OnTagClear();
	afx_msg void OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClear();
	afx_msg void OnCopy();
	afx_msg void OnCopyAll();
	afx_msg void OnBnClickedAdd();
};
