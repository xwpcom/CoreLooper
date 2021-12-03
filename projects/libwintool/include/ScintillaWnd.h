#pragma once

//XiongWanPing 2019.09.14
//参考https://blog.csdn.net/embededvc/article/details/7049788
//语法高亮编辑控件Scintilla在MFC中的简单使用
//https://www.scintilla.org/ScintillaDownload.html
class WIN_CLASS ScintillaWnd : public CWnd
{
	DECLARE_DYNAMIC(ScintillaWnd)

public:
	ScintillaWnd();
	virtual ~ScintillaWnd();
	BOOL Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void SetReadOnly(bool readOnly);
	long currentPos();
	long textLength();
	void clearStyle();
	int RegisterStyle(COLORREF textColor,COLORREF backColor=RGB(255,255,255));
	int SetStyle(int style,long start, long length);
	string body();
protected:
	DECLARE_MESSAGE_MAP()
	int mNextStyleId = 88;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
