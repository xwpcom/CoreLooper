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
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
