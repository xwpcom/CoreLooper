// MySqlDemo.h : main header file for the MYSQLDEMO application
//

#if !defined(AFX_MYSQLDEMO_H__17BEEF49_8F7A_4F5E_B27F_71FA90608C9A__INCLUDED_)
#define AFX_MYSQLDEMO_H__17BEEF49_8F7A_4F5E_B27F_71FA90608C9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMySqlDemoApp:
// See MySqlDemo.cpp for the implementation of this class
//

class CMySqlDemoApp : public CWinApp
{
public:
	CMySqlDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySqlDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMySqlDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSQLDEMO_H__17BEEF49_8F7A_4F5E_B27F_71FA90608C9A__INCLUDED_)
