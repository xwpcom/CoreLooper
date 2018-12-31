// libhttp.h : main header file for the libhttp DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ClibhttpApp
// See libhttp.cpp for the implementation of this class
//

class ClibhttpApp : public CWinApp
{
public:
	ClibhttpApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
