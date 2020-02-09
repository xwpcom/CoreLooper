#pragma once


class ConsolePage; 
using namespace std;
class ChildView : public CWnd
{
// Construction
public:
	ChildView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~ChildView();

	
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	void OnSetFocus(CWnd* pOldWnd);

	shared_ptr<ConsolePage> mConsolePage;
	void UpdateControlPos();


public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

