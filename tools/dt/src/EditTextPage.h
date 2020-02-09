#pragma once


// EditTextPage dialog

class EditTextPage : public BasePage
{
	DECLARE_DYNAMIC(EditTextPage)

public:
	EditTextPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~EditTextPage();

	CString mTitle;
	CString mText;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EditTextPage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
