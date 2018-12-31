#pragma once

//XiongWanPing 2016.10.14
//名称来源于Android Toast,用来显示临时信息，一般是透明，会自动消失
class WIN_CLASS ToastWnd : public CWnd
{
	DECLARE_DYNAMIC(ToastWnd)

public:
	ToastWnd();
	virtual ~ToastWnd();

	BOOL Create(CRect rc, DWORD dwStyle = WS_CLIPCHILDREN | WS_VISIBLE | WS_OVERLAPPED);
	int SetAlpha(int alpha);
	virtual void SetText(const CString& text);
	void Show(CWnd *wnd,CString text, DWORD msDisplay = 500);

protected:
	DECLARE_MESSAGE_MAP()
	enum
	{
		eTimerStartFadeOut,
		eTimerFadeOut,		//逐隐
	};


	BOOL RegisterClass();
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* pDC);

	CRect Draw(CDC& dc, bool onlyCalc = false);
	CString mText;

	int mBaseAlpha;
	int mCurrentAlpha;
	CWnd *mBuddy;

public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


