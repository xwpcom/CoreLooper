#if !defined(AFX_OPTIONGENERALDLG_H__4D7BF060_A670_4C87_AF49_00D6F481864F__INCLUDED_)
#define AFX_OPTIONGENERALDLG_H__4D7BF060_A670_4C87_AF49_00D6F481864F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionGeneralDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionGeneralDlg dialog

class COptionGeneralDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionGeneralDlg)

// Construction
public:
	COptionGeneralDlg();
	~COptionGeneralDlg();

// Dialog Data
	//{{AFX_DATA(COptionGeneralDlg)
	enum { IDD = IDD_GENERAL };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionGeneralDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionGeneralDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONGENERALDLG_H__4D7BF060_A670_4C87_AF49_00D6F481864F__INCLUDED_)
