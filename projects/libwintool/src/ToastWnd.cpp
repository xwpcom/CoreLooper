#include "stdafx.h"
#include "../include/ToastWnd.h"

#define CLASS_WND _T("ToastWnd")
using namespace Bear::Core;

IMPLEMENT_DYNAMIC(ToastWnd, CWnd)

ToastWnd::ToastWnd()
{
	mBaseAlpha = 160;
	mCurrentAlpha = mBaseAlpha;
	mBuddy = nullptr;
}

ToastWnd::~ToastWnd()
{
}

BEGIN_MESSAGE_MAP(ToastWnd, CWnd)
	ON_WM_PAINT()

	ON_WM_NCHITTEST()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL ToastWnd::RegisterClass()
{
	{
		WNDCLASS wc = { 0 };
		wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ::DefWindowProc;
		wc.hInstance = AfxGetInstanceHandle();
		wc.lpszClassName = CLASS_WND;
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		AfxRegisterClass(&wc);
	}

	return TRUE;
}

BOOL ToastWnd::Create(CRect rc, DWORD dwStyle)
{
	BOOL bOK = CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CLASS_WND, _T("ToastWnd"), dwStyle, rc, nullptr, 0);
	SetAlpha(100);
	return bOK;
}

int ToastWnd::SetAlpha(int alpha)
{
	mCurrentAlpha = alpha;

	HWND hWnd = m_hWnd;

	typedef DWORD(WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);

	HMODULE hDLL = LoadLibrary(_T("user32"));
	auto pSetLayeredWindowAttributes = (PSLWA)GetProcAddress(hDLL, "SetLayeredWindowAttributes");
	if (pSetLayeredWindowAttributes != NULL)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong
		(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED/*|WS_EX_TOOLWINDOW*/);

		//0 means transparent
		//255 means Opaque
		pSetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), alpha, LWA_COLORKEY | LWA_ALPHA);
	}

	return 0;
}

BOOL ToastWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	{
		static bool first = true;
		if (first)
		{
			first = false;
			RegisterClass();
		}
	}

	return CWnd::PreCreateWindow(cs);
}

void ToastWnd::OnPaint()
{
	CPaintDC dc(this);
	Draw(dc);
}

BOOL ToastWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

LRESULT ToastWnd::OnNcHitTest(CPoint point)
{
	return HTCAPTION;

	//return CWnd::OnNcHitTest(point);
}

void ToastWnd::OnDestroy()
{
	//DW("ToastWnd::OnDestroy()=========================");
	CWnd::OnDestroy();
}

CRect ToastWnd::Draw(CDC& dc, bool onlyCalc)
{
	CRect rc;
	GetClientRect(rc);

	dc.FillSolidRect(rc, RGB(255, 255, 255));
	dc.SetTextColor(RGB(0,255, 0));
	dc.SetBkMode(TRANSPARENT);

	int gap = 16;

	{
		CPen m_BoundryPen;
		m_BoundryPen.CreatePen(PS_SOLID, 5, RGB(255, 255, 255));
		CPen* hOldPen = dc.SelectObject(&m_BoundryPen);

		CRect rect;
		GetClientRect(&rect);
		CBrush brush, brushOut;
		brush.CreateSolidBrush(RGB(0x4D, 0x4D, 0x4D));
		dc.SelectObject(&brush);
		dc.RoundRect(rect.left, rect.top, rect.right, rect.bottom, gap, gap);
		dc.SelectObject(GetStockObject(NULL_BRUSH));
		dc.SelectObject(hOldPen);
	}

	CFont font;
	font.CreatePointFont(160, _T("新宋体"));
	CFont *oldFont = dc.SelectObject(&font);

	DWORD flags = DT_LEFT | DT_VCENTER;
	CRect rcText = rc;
	if (onlyCalc)
	{
		flags |= DT_CALCRECT;
	}
	else
	{
		rcText.OffsetRect(gap / 2, gap / 2);
	}
	int h = dc.DrawText(mText, rcText, flags);
	dc.SelectObject(oldFont);

	if (onlyCalc)
	{
		rcText.bottom = rcText.top + h + gap;
		rcText.right += gap;
	}

	return rcText;
}

void ToastWnd::SetText(const CString& text)
{
	//auto resize
	mText = text;

	CDC *pDC = GetDC();
	CRect rc = Draw(*pDC, true);
	ReleaseDC(pDC);

	ASSERT(mBuddy);
	CRect rcBuddy;
	mBuddy->GetWindowRect(rcBuddy);
	//在mBuddy上面居中显示
	int dx = (rcBuddy.Width() - rc.Width()) / 2;
	int dy = (rcBuddy.Height() - rc.Height()) / 2;
	dx = MAX(0, dx);
	dy = MAX(0, dy);
	rc.OffsetRect(rcBuddy.left+dx, rcBuddy.top+dy);
	MoveWindow(rc);

	Invalidate();
}

void ToastWnd::Show(CWnd *wnd, CString text, DWORD msDisplay)
{
	mBuddy = wnd;

	KillTimer(eTimerStartFadeOut);
	KillTimer(eTimerFadeOut);
	ShowWindow(SW_SHOW);
	SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	SetText(text);
	SetAlpha(mBaseAlpha);
	SetTimer(eTimerStartFadeOut, msDisplay,nullptr);
}

void ToastWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimerStartFadeOut:
	{
		SetTimer(eTimerFadeOut, 100, nullptr);
		break;
	}
	case eTimerFadeOut:
	{
		int alpha = MAX(mCurrentAlpha -30,0);
		if (alpha == 0)
		{
			KillTimer(eTimerFadeOut);
		}
		
		SetAlpha(alpha);
		break;
	}
	}

	CWnd::OnTimer(nIDEvent);
}
