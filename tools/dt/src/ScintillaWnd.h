#pragma once
#include "Scintilla.h"

//XiongWanPing 2019.09.14
//参考https://blog.csdn.net/embededvc/article/details/7049788
//语法高亮编辑控件Scintilla在MFC中的简单使用
//https://www.scintilla.org/ScintillaDownload.html
class ScintillaWnd : public CWnd
{
	DECLARE_DYNAMIC(ScintillaWnd)

public:
	ScintillaWnd();
	virtual ~ScintillaWnd();
	BOOL Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void SetReadOnly(bool readOnly)
	{
		SendMessage(SCI_SETREADONLY, readOnly);
	}

	void SetCodePage(int code);
	int GetCodePage()const
	{
		return mCodePage;
	}
protected:
	DECLARE_MESSAGE_MAP()
	int mCodePage=SC_CP_UTF8;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
