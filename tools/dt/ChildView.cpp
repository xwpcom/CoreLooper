#include "pch.h"
#include "framework.h"
#include "dt.app.h"
#include "ChildView.h"
#include "src/ConsolePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

ChildView::ChildView()
{
}

ChildView::~ChildView()
{
}


BEGIN_MESSAGE_MAP(ChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// ChildView message handlers

BOOL ChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style |= WS_CLIPCHILDREN;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void ChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
}

int ChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	auto obj = make_shared<ConsolePage>();
	obj->Create(IDD_Console, this);
	mConsolePage = obj;

	UpdateControlPos();
	obj->ShowWindow(SW_SHOW);

	return 0;
}


void ChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	UpdateControlPos();
}

void ChildView::UpdateControlPos()
{
	auto obj = mConsolePage;
	if (obj && obj->GetSafeHwnd())
	{
		CRect rc;
		GetClientRect(rc);
		obj->MoveWindow(rc);
	}
}

void ChildView::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	if (mConsolePage)
	{
		mConsolePage->SetFocus();
	}
}
