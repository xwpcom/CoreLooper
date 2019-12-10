// libdb.h : main header file for the libdb DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ClibdbApp
// See libdb.cpp for the implementation of this class
//

class ClibdbApp : public CWinApp
{
public:
	ClibdbApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
