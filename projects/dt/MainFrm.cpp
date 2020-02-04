// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DebugHelper.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDT_UPDATE_CURRENT_POS	1

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_TOP_MOST, OnTopMost)
	ON_UPDATE_COMMAND_UI(ID_TOP_MOST, OnUpdateTopMost)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_EXIT, OnEditExit)
	ON_COMMAND(ID_MIN, OnMin)
	ON_WM_COPYDATA()
	ON_UPDATE_COMMAND_UI(ID_SOCKET, OnUpdateSocket)
	ON_COMMAND(ID_SOCKET, OnSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_TIME,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bTopMost = FALSE;
	m_bRecvSocketMsg = TRUE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);
	
	//LoadBarState ("Bars\\Settings");
	
	m_bTopMost = AfxGetApp()->GetProfileInt("Settings","TopMost",TRUE);
	if(m_bTopMost)
		SetWindowPos(&wndTopMost,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
	else
		SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

	m_bRecvSocketMsg = AfxGetApp()->GetProfileInt("Settings","m_bRecvSocketMsg",FALSE);

	//m_wndToolBar.SetSizes(CSize(72,36),CSize(16,15));
	/*
	m_wndToolBar.SetSizes(CSize(72,36),CSize(16,15));
	m_wndToolBar.SetButtonText(m_wndToolBar.CommandToIndex(ID_EDIT_EXIT),"退出");
	m_wndToolBar.SetButtonText(m_wndToolBar.CommandToIndex(ID_FILE_SAVE),"保存记录");
	m_wndToolBar.SetButtonText(m_wndToolBar.CommandToIndex(ID_EDIT_CLEARALL),"清空列表");
	m_wndToolBar.SetButtonText(m_wndToolBar.CommandToIndex(ID_TOP_MOST),"保持最前");
	m_wndToolBar.SetButtonText(m_wndToolBar.CommandToIndex(ID_LOG),"写入注册表");
	//*/
	
	SetTimer(IDT_UPDATE_CURRENT_POS,200,NULL);
	SetTimeText();

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style &= ~FWS_ADDTOTITLE;
	cs.hMenu = NULL;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if(message == WM_SETTEXT && GetActiveView())
	{
		ReplyMessage(TRUE);

		CString szMsg = (LPCTSTR)lParam;
		((CDebugHelperView*)GetActiveView())->AddString(szMsg);
		return 1;
	}
	else if(message == WM_COPYDATA)
	{
		COPYDATASTRUCT *pcs = (COPYDATASTRUCT*)lParam;
		CString szMsg = (LPCTSTR)pcs->lpData;
		((CDebugHelperView*)GetActiveView())->AddString(szMsg);
		return 1;
	}
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

void CMainFrame::OnTopMost() 
{
	m_bTopMost = !m_bTopMost;
	if(m_bTopMost)
		SetWindowPos(&wndTopMost,0,0,0,0,SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE|SWP_NOACTIVATE);
	else
		SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
	
}

void CMainFrame::OnUpdateTopMost(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTopMost);	
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == IDT_UPDATE_CURRENT_POS)
	{
		SetTimeText();
		/*
		if(m_bTopMost)
		{
			SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
		}
		//*/

	}
	CFrameWnd::OnTimer(nIDEvent);
}
//DT is a macro to help debug app by dump useful info,
//it works for debug and release version.
//usage:
//DT("work ok in line %d",m_nLine);

//define following line to enable DT
#define DT	DebugTrace
//uncomment following line to disable DT
//#undef DT
//#define DT

BOOL DebugTrace(char * lpszFormat,...)
{
	static HWND hwnd = ::FindWindowA(NULL,"DebugHelper ");
	if(!IsWindow(hwnd))
		hwnd = ::FindWindowA(NULL,"DebugHelper ");
	if(hwnd)
	{
		static char szMsg[512];

		va_list argList;
		va_start(argList, lpszFormat);
		try
		{
			vsprintf(szMsg,lpszFormat, argList);
		}
		catch(...)
		{
			strcpy(szMsg ,"DebugHelper:Invalid string format!");
		}
		va_end(argList);

		::SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szMsg);
	}
	return TRUE;
}

void CMainFrame::SetTimeText()
{
	CTime tm = CTime::GetCurrentTime();
	CString szTime = tm.Format("%H:%M:%S");
	{
		CDebugHelperView *pView = (CDebugHelperView*)GetActiveView();
		if(pView)
		{
			CListCtrl& lstCtrl = pView->GetListCtrl();
			int nSel = lstCtrl.GetNextItem(-1,LVNI_SELECTED);
			int nc = lstCtrl.GetItemCount();
			CString szPos;
			szPos.Format("  (%d:%d)",nSel+1,nc);
			szTime += szPos;
		}
	}
	m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_SEPARATOR),szTime);
	
	//DT(szTime.GetBufferSetLength(MAX_PATH));
	//DT("Test");
}

void CMainFrame::OnDestroy() 
{
//	SaveBarState (_T("Bars\\Settings"));	

	// save windows position info
	WINDOWPLACEMENT wndStatus = {0};
	wndStatus.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wndStatus);
	AfxGetApp()->WriteProfileBinary("Settings","WindowPos",
		(LPBYTE)&wndStatus,sizeof(wndStatus));
	AfxGetApp()->WriteProfileInt("Settings","TopMost",m_bTopMost);
	AfxGetApp()->WriteProfileInt("Settings","m_bRecvSocketMsg",m_bRecvSocketMsg);
	
	CFrameWnd::OnDestroy();
}

void CMainFrame::ActivateFrame(int nCmdShow) 
{
	static BOOL bFirst = TRUE;
	if(bFirst)
	{
		bFirst = FALSE;
		return;
	}
	CFrameWnd::ActivateFrame(nCmdShow);
}

void CMainFrame::OnEditExit() 
{
	SendMessage(WM_CLOSE);
	
}

void CMainFrame::OnMin() 
{
	ShowWindow(SW_MINIMIZE);	
}

BOOL CMainFrame::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	COPYDATASTRUCT *pCS = pCopyDataStruct;

	return CFrameWnd::OnCopyData(pWnd, pCopyDataStruct);
}

void CMainFrame::OnUpdateSocket(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bRecvSocketMsg);
	
}

void CMainFrame::OnSocket() 
{
	m_bRecvSocketMsg = !m_bRecvSocketMsg;	
}
