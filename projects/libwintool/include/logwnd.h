#pragma once
#include "ScintillaWnd.h"

//XiongWanPing 2019.09.17
class WIN_CLASS LogWnd :public ScintillaWnd
{
	DECLARE_DYNAMIC(LogWnd)
public:
	LogWnd();
	void AddLog(const string& line);
	void clear();
	void CopyAll();
	void CheckUpdateMarginWidth();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

};
