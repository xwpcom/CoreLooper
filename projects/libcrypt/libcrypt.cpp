// libcrypt.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "libcrypt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ClibcryptApp, CWinApp)
END_MESSAGE_MAP()


// ClibcryptApp construction

ClibcryptApp::ClibcryptApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only ClibcryptApp object

ClibcryptApp theApp;


// ClibcryptApp initialization

BOOL ClibcryptApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
