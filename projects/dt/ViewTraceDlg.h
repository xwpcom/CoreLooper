#if !defined(AFX_VIEWTRACEDLG_H__AEB7AAAF_8CF4_44D4_B84E_7BAE4B3567D9__INCLUDED_)
#define AFX_VIEWTRACEDLG_H__AEB7AAAF_8CF4_44D4_B84E_7BAE4B3567D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewTraceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewTraceDlg dialog

class CViewTraceDlg : public CDialog
{
// Construction
public:
	void SetIndex(int nIndex);
	int m_nLine;
	CViewTraceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewTraceDlg)
	enum { IDD = IDD_VIEW_DLG };
	CString	m_szTime;
	CString	m_szData;
	CString	m_szLine;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewTraceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CViewTraceDlg)
	afx_msg void OnBtnUp();
	afx_msg void OnBtnDown();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWTRACEDLG_H__AEB7AAAF_8CF4_44D4_B84E_7BAE4B3567D9__INCLUDED_)
