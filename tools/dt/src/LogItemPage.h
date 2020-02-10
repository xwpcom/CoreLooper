#pragma once
#include "loginfo.h"
class LogWnd;

class LogItemPage : public BasePage
{
	DECLARE_DYNAMIC(LogItemPage)

public:
	LogItemPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~LogItemPage();
	
	void SetLogItem(LogItem*);
	void clear();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LogItemPage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	int Init();
	void OnRelayout(const CRect& rc)override;

	shared_ptr<LogWnd> mEdit;

public:
	afx_msg void OnBnClickedCopy();
};
