// DebugHelper.h : main header file for the DEBUGHELPER application
//

#if !defined(AFX_DEBUGHELPER_H__3C8EB469_E18B_42CA_9935_932F5B332109__INCLUDED_)
#define AFX_DEBUGHELPER_H__3C8EB469_E18B_42CA_9935_932F5B332109__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperApp:
// See DebugHelper.cpp for the implementation of this class
//

class CDebugHelperApp : public CWinApp
{
public:
	CDebugHelperApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugHelperApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CDebugHelperApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HANDLE m_hMutex;

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGHELPER_H__3C8EB469_E18B_42CA_9935_932F5B332109__INCLUDED_)
