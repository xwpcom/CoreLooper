// libcrypt.h : main header file for the libcrypt DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ClibcryptApp
// See libcrypt.cpp for the implementation of this class
//

class ClibcryptApp : public CWinApp
{
public:
	ClibcryptApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
