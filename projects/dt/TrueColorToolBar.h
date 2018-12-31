#ifndef TRUECOLORTOOLBAR_H_
#define TRUECOLORTOOLBAR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CTrueColorToolBar

class CTrueColorToolBar : public CToolBar
{
// Construction
public:
	CTrueColorToolBar();
	void _GetButton(int nIndex, TBBUTTON* pButton) const
	{
		CToolBar::_GetButton(nIndex,pButton);
	}
// Attributes
private:
	BOOL m_bDropDown;

	struct stDropDownInfo {
	public:
		UINT  uButtonID;
		UINT  uMenuID;
		CWnd* pParent;
	};
	
	CArray <stDropDownInfo, stDropDownInfo&> m_lstDropDownButton;
	
// Operations
public:
	BOOL LoadTrueColorToolBar(int  nBtnWidth,
							  UINT uToolBar,
							  UINT uToolBarHot		= 0,
							  UINT uToolBarDisabled = 0);

	void AddDropDownButton(CWnd* pParent, UINT uButtonID, UINT uMenuID);

private:
	BOOL SetTrueColorToolBar(UINT uToolBarType,
		                     UINT uToolBar,
						     int  nBtnWidth);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrueColorToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTrueColorToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTrueColorToolBar)
#if _MSC_VER>1300
	afx_msg void OnToolbarDropDown(NMHDR * pnmh, LRESULT* plRes);
#else	
	afx_msg void OnToolbarDropDown(NMTOOLBAR* pnmh, LRESULT* plRes);
#endif
	afx_msg void OnNcPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // TRUECOLORTOOLBAR_H_
