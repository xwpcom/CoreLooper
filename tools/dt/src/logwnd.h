#pragma once
#include "ScintillaWnd.h"

//XiongWanPing 2019.09.17
class LogWnd :public ScintillaWnd
{
	DECLARE_DYNAMIC(LogWnd)
public:
	LogWnd();
	void AddLog(const string& line);
	void clear();
	void CopyAll();
	void SetTextColor(COLORREF color);
protected:
	DECLARE_MESSAGE_MAP()
	void CheckUpdateMarginWidth();
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

};
