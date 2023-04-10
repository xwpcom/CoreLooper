#include "stdafx.h"
#include "../include/ScintillaWnd.h"
#include "Scintilla.h"
#include "SciLexer.h"

using namespace Bear::Core;
IMPLEMENT_DYNAMIC(ScintillaWnd, CWnd)

ScintillaWnd::ScintillaWnd()
{

}

ScintillaWnd::~ScintillaWnd()
{
}

int ScintillaWnd::SetStyle(int style, long start, long length)
{
	SendMessage(SCI_STARTSTYLING, start, 0);
	return SendMessage(SCI_SETSTYLING, length, style);
}

void ScintillaWnd::clearStyle()
{
	SendMessage(SCI_STYLECLEARALL);
	mNextStyleId = 88;
}

int ScintillaWnd::RegisterStyle(COLORREF textColor, COLORREF backColor)
{
	auto id = mNextStyleId++;
	SendMessage(SCI_STYLESETFORE, id, textColor);
	SendMessage(SCI_STYLESETBACK, id, backColor);//test ok
	return id;
}

BEGIN_MESSAGE_MAP(ScintillaWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// ScintillaWnd message handlers

BOOL ScintillaWnd::Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	HMODULE mSci = nullptr;
#ifdef _DEBUG
	auto dll = _T("SciLexerD.dll");
#else
	auto dll = _T("SciLexer.dll");
#endif
	mSci = LoadLibrary(dll);

	auto ret=CWnd::CreateEx(dwExStyle, _T("Scintilla"), _T(""), dwStyle, rect, pParentWnd, nID);
	return ret;
}

void ScintillaWnd::setFontSize(int size)
{
	mFontSize = size;
	if (GetSafeHwnd())
	{
		SendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, (LPARAM)mFontSize);//字体大小
	}
}

int ScintillaWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SendMessage(SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE);

	SendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)_T("新宋体"));
	SendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, (LPARAM)mFontSize);//字体大小

	SendMessage(SCI_SETTABWIDTH, 4);
	SendMessage(SCI_SETBACKSPACEUNINDENTS, true);//?

	//SendMessage(SCI_SETMARGINBACKN, SC_MARGIN_BACK, RGB(0x61, 0xB3, 0x4D));
	//SendMessage(SCI_STYLESETWEIGHT, STYLE_DEFAULT, 1000);//not work
	//SendMessage(EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));//not work,doc中明确说不支持

	return 0;
}

void ScintillaWnd::OnTimer(UINT_PTR nIDEvent)
{

	CWnd::OnTimer(nIDEvent);
}

void ScintillaWnd::SetReadOnly(bool readOnly)
{
	SendMessage(SCI_SETREADONLY, readOnly);
}

long ScintillaWnd::currentPos()
{
	return SendMessage(SCI_GETCURRENTPOS);
}

long ScintillaWnd::textLength()
{
	return SendMessage(SCI_GETTEXTLENGTH);
}

string ScintillaWnd::body()
{
	string text;
	auto bytes = textLength();
	if (bytes > 0)
	{
		text.reserve(bytes+1);
		text.resize(bytes);
		SendMessage(SCI_GETTEXT, bytes, (LPARAM)(char*)text.data());
	}

	return text;
}

