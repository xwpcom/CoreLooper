#pragma once
#include "BasePage.h"

struct tagPageInfo
{
	shared_ptr<BasePage>	mPage;
	UINT					mIdd;
	CString					mTitle;
};

class WIN_CLASS CategoryPage : public BasePage
{
	DECLARE_DYNAMIC(CategoryPage)

public:
	CategoryPage(UINT idd,CWnd* pParent = NULL);   // standard constructor
	virtual ~CategoryPage();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void UpdateControlPos();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	int Init();
	virtual vector<tagPageInfo> GetPages() = 0;

	CMFCTabCtrl	 mTab;
	std::map<int, shared_ptr<BasePage>> mGroups;

	CRect mTabDeflateRect=CRect(0,4,0,0);
};
