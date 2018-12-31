// MsgSinkWnd.cpp : implementation file
//

#include "stdafx.h"
#include "MsgSinkWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLASS_MSG_SINK		"CMsgSink"
/////////////////////////////////////////////////////////////////////////////
// CMsgSinkWnd

CMsgSinkWnd::CMsgSinkWnd()
{
	m_pWndMsgReceiver = NULL;
}

CMsgSinkWnd::~CMsgSinkWnd()
{
	::DestroyWindow(m_hWnd);
}


BEGIN_MESSAGE_MAP(CMsgSinkWnd, CWnd)
	//{{AFX_MSG_MAP(CMsgSinkWnd)
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SINK_MSG,OnSinkMsg)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMsgSinkWnd message handlers

BOOL CMsgSinkWnd::Create(CString szName)
{
	return CreateEx(NULL,CLASS_MSG_SINK,szName,0,0,0,0,0,HWND_MESSAGE,0,0);
}

BOOL CMsgSinkWnd::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	COPYDATASTRUCT* pCDS = pCopyDataStruct;
	
	DWORD dwData = pCDS->dwData;
	return CWnd::OnCopyData(pWnd, pCopyDataStruct);
}

BOOL CMsgSinkWnd::Register()
{
	static BOOL bRegistered = FALSE;
	if(!bRegistered)
	{
		WNDCLASS wc = {0};
		wc.style         = NULL;
		wc.lpfnWndProc   = ::DefWindowProc;
		wc.hInstance     = AfxGetInstanceHandle();
		wc.lpszClassName = CLASS_MSG_SINK;
		wc.hbrBackground = NULL;
		wc.hCursor       = NULL;
		bRegistered = AfxRegisterClass(&wc);
	}
	return bRegistered;
}

BOOL CMsgSinkWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(!Register())
		return FALSE;

	return CWnd::PreCreateWindow(cs);
}


void CMsgSinkWnd::SetMsgReceiver(CWnd *pWnd)
{
	m_pWndMsgReceiver = pWnd;
}

LRESULT CMsgSinkWnd::OnSinkMsg(WPARAM wp,LPARAM lp)
{
	if(m_pWndMsgReceiver)
	{
		m_pWndMsgReceiver->SendMessage(WM_COMMAND,wp,lp);

	}
	return 0;
}

LRESULT CMsgSinkWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if(message == WM_SETTEXT && m_pWndMsgReceiver)
	{
		m_pWndMsgReceiver->SendMessage(WM_SETTEXT,0,lParam);
		return  1;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}
