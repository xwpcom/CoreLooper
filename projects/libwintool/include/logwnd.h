#pragma once
#include "ScintillaWnd.h"

//XiongWanPing 2019.09.17
class WIN_CLASS LogWnd :public ScintillaWnd
{
	DECLARE_DYNAMIC(LogWnd)
public:
	LogWnd();
	int AddLog(const string& line);
	int addLine(const string& line)
	{
		return AddLog(line + "\r\n");
	}

	int addLogFast(const string& line);
	void clear();
	void CopyAll();
	void SetUtf8();
	void SetGB();
	void CheckUpdateMarginWidth();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

};
