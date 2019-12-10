// MySqlDemoDlg.h : header file
//

#if !defined(AFX_MYSQLDEMODLG_H__6795FDEA_ED92_4A01_AA5B_68178C797CD7__INCLUDED_)
#define AFX_MYSQLDEMODLG_H__6795FDEA_ED92_4A01_AA5B_68178C797CD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMySqlDemoDlg dialog

class CMySqlDemoDlg : public CDialog
{
// Construction
public:
	CMySqlDemoDlg(CWnd* pParent = NULL);	// standard constructor

	MYSQL mysql; //数据库连接句柄

	CListCtrl m_list;
	int m_nIndex;
	CString str_PreName;
	void RefreshList();
// Dialog Data
	//{{AFX_DATA(CMySqlDemoDlg)
	enum { IDD = IDD_MYSQLDEMO_DIALOG };

	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySqlDemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMySqlDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnButton_Update();
	afx_msg void OnButton_Delete();
	afx_msg void OnButton_Add();
	afx_msg void OnClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSQLDEMODLG_H__6795FDEA_ED92_4A01_AA5B_68178C797CD7__INCLUDED_)
	
