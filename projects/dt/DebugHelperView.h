#if !defined(AFX_DEBUGHELPERVIEW_H__D209BE53_B815_4FE4_8148_0D77BE1BE201__INCLUDED_)
#define AFX_DEBUGHELPERVIEW_H__D209BE53_B815_4FE4_8148_0D77BE1BE201__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DebugHelperDoc.h"
#include <afxcview.h>
#include <afxTempl.h>

#include "ViewTraceDlg.h"

#include "MsgSinkWnd.h"
struct tagMsgEx
{
	CString msg;
	int level;
};

enum
{
	eIdxMsg,
	eIdxTime,
	eIdxFileLine,
	eIdxPid,
};


class CDebugHelperView : public CListView
{
protected: // create from serialization only
	CDebugHelperView();
	DECLARE_DYNCREATE(CDebugHelperView)

// Attributes
public:
	CDebugHelperDoc* GetDocument();

// Operations
public:
	int mArrIdx[10];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugHelperView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void AddString(CString szMsg);
	virtual ~CDebugHelperView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	LRESULT OnSocketMsg(WPARAM wp,LPARAM lp);
	void OnCustomdrawList ( NMHDR* pNMHDR, LRESULT* pResult );
	//{{AFX_MSG(CDebugHelperView)
	afx_msg void OnEditClearAll();
	afx_msg void OnUpdateEditClearAll(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnFileSave();
	afx_msg void OnLog();
	afx_msg void OnUpdateLog(CCmdUI* pCmdUI);
	afx_msg void OnTest();
	afx_msg void OnDestroy();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnOption();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static UINT WINAPI _RecvThreadCB(void *pThis);
	UINT _RecvThread();


	BOOL SaveToFile(BOOL bShowMsgBox = TRUE);
	CFont m_font;
	DWORD m_dwLine;

	CStringList m_lstString;
	CViewTraceDlg *m_pViewTraceDlg;
	int m_nTimeSave;//定时保存的时间间隔,单位为秒.

	BOOL m_bLog;//是否保存输出到注册表

	CMsgSinkWnd m_wndMsgSink;

	UINT m_nMaxLine;
	HANDLE m_hThread;
};

#ifndef _DEBUG  // debug version in DebugHelperView.cpp
inline CDebugHelperDoc* CDebugHelperView::GetDocument()
   { return (CDebugHelperDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGHELPERVIEW_H__D209BE53_B815_4FE4_8148_0D77BE1BE201__INCLUDED_)
