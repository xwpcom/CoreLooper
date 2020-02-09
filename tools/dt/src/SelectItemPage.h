#pragma once


#include "basepage.h"

class SelectItemPage : public BasePage
{
	DECLARE_DYNAMIC(SelectItemPage)

public:
	SelectItemPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~SelectItemPage();

	string mTitle;
	string mItemTitle;
	unordered_map<string, bool> mItems;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SelectItemPage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	int Init();
public:
	CListCtrl mListCtrl;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedSelectAll();
	afx_msg void OnBnClickedSelectInvert();
};
