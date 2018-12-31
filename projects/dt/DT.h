#ifndef _DEBUG_TRACE_2004
#define _DEBUG_TRACE_2004

#if defined __AFX_H__

#include "resource.h"
#include "Shlwapi.h"
#pragma comment(lib,"shlwapi.lib")

//prefix Sfx.. is Sure's prefix,sure is me.
//create a path
BOOL SfxCreatePath(CString szPath,int nStartPos = 2);
BOOL SfxIsDirectory(CString szPath);
UINT SfxBrowseFolder(CString &szFolder,LPCTSTR  lpszDisplay = NULL,CWnd *pParentWnd = 0,UINT uFlags = 0);
CString SfxGetModulePath(HINSTANCE hInst = NULL);

UINT SfxTimedMessageBox(HWND hwndParent,LPCTSTR ptszMessage,LPCTSTR ptszTitle,UINT nIDDefault,
						UINT flags = MB_OK,DWORD dwTimeout = 10*1000);
UINT SfxTimedMessageBox(HWND hwndParent,UINT nIDMessage,UINT nIDTitle,UINT nIDDefault,
						UINT flags = MB_OK,DWORD dwTimeout = 10*1000);
typedef HRESULT (_stdcall *PSHAutoComplete)(HWND hwndEdit,DWORD dwFlags);


// Currently (SHACF_FILESYSTEM | SHACF_URLALL)
#define SHACF_DEFAULT                   0x00000000

// This includes the File System as well as the rest of the shell (Desktop\My Computer\Control Panel\)
#define SHACF_FILESYSTEM                0x00000001

#define SHACF_URLALL                    (SHACF_URLHISTORY | SHACF_URLMRU)

// URLs in the User's History
#define SHACF_URLHISTORY                0x00000002

// URLs in the User's Recently Used list.
#define SHACF_URLMRU                    0x00000004
  
// Use the tab to move thru the autocomplete possibilities instead of to the next dialog/window control.
#define SHACF_USETAB                    0x00000008  

// This includes the File System
#define SHACF_FILESYS_ONLY              0x00000010  

// Same as SHACF_FILESYS_ONLY except it only includes directories, UNC servers, and UNC server shares.
#define SHACF_FILESYS_DIRS              0x00000020  

// Ignore the registry default and force the feature on.
#define SHACF_AUTOSUGGEST_FORCE_ON      0x10000000  

// Ignore the registry default and force the feature off.
#define SHACF_AUTOSUGGEST_FORCE_OFF     0x20000000  

// Ignore the registry default and force the feature on. (Also know as AutoComplete)
#define SHACF_AUTOAPPEND_FORCE_ON       0x40000000  

// Ignore the registry default and force the feature off. (Also know as AutoComplete)
#define SHACF_AUTOAPPEND_FORCE_OFF      0x80000000  

HRESULT	SfxSHAutoComplete(HWND hWnd,DWORD dwFlags = SHACF_FILESYS_ONLY);
HRESULT	SfxSHAutoComplete(CWnd *pWnd,DWORD dwFlags = SHACF_FILESYS_ONLY);

//
#define PS_DRIVE		0x0001
#define PS_DIR			0x0002
#define PS_FILENAME		0x0003
#define PS_TITLE		0x0004
#define PS_EXT			0x0005


CString SfxGetPathSplit(LPCTSTR szFullFileName,DWORD dwFlags);
BOOL SfxChangeShellLink(LPCTSTR szLink,LPCTSTR szDest,DWORD dwFlags = 0);
//
#endif
#define DEBUG_TRACE
//只在release下并且也定义了DEBUG_TRACE的情况下输出到文件
#ifndef _DEBUG
	#define	DEBUG_TRACE_FILE
#endif

#ifdef DEBUG_TRACE
	BOOL DebugTrace(LPCTSTR lpszFormat,...);
	#define DT	DebugTrace
	#define _DT DEBUG_TRACE
#else
	#define DT
	#undef _DT
#endif


#endif