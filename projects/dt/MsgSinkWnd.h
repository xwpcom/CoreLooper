#if !defined(AFX_MSGSINKWND_H__CE549907_63ED_4F1E_A301_D521F1A9E8C5__INCLUDED_)
#define AFX_MSGSINKWND_H__CE549907_63ED_4F1E_A301_D521F1A9E8C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgSinkWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMsgSinkWnd window

/*
Message-only windows
//*/
#define WM_SINK_MSG	(WM_USER+1)
class CMsgSinkWnd : public CWnd
{
// Construction
public:
	CMsgSinkWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgSinkWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetMsgReceiver(CWnd *pWnd);
	BOOL Create(CString szName);
	virtual ~CMsgSinkWnd();

	// Generated message map functions
protected:
	LRESULT OnSinkMsg(WPARAM wp,LPARAM lp);
	CWnd *m_pWndMsgReceiver;//当CMsgSinkWnd接收到消息后向此CWnd转发此消息.
	BOOL Register();
	//{{AFX_MSG(CMsgSinkWnd)
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGSINKWND_H__CE549907_63ED_4F1E_A301_D521F1A9E8C5__INCLUDED_)
