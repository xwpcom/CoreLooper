// libwintool.h : main header file for the libwintool DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ClibwintoolApp
// See libwintool.cpp for the implementation of this class
//

class ClibwintoolApp : public CWinApp
{
public:
	ClibwintoolApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
