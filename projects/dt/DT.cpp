#include "stdafx.h"
#include "DT.h"
#include "resource.h"

#if defined __AFX_H__
	#include <AFXPRIV.H>
	#include <afxmt.h>
	int g_nTimedMsgBox = 10;
	BOOL g_bInitCaption = TRUE;
#endif

#ifdef DEBUG_TRACE
#include <stdio.h>
BOOL DebugTrace(LPCTSTR lpszFormat,...)
{
#if defined __AFX_H__
	//提供一种方法让用户取消DebugTrace
	{
		static BOOL bTrace = AfxGetApp()->GetProfileInt("Settings","DebugTrace",TRUE);
		if(!bTrace)
			return TRUE;
	}

	//格式化字符串
	CString szMsg;
	va_list argList;
	va_start(argList, lpszFormat);
	try
	{
		szMsg.FormatV(lpszFormat, argList);
	}
	catch(...)
	{
		szMsg = "DebugHelper输出字符串格式错误!";
	}
	va_end(argList);
	
	ASSERT(AfxIsValidString(lpszFormat));
	
	static HWND hwnd = ::FindWindow(NULL,_T("DebugHelper "));
	if(!IsWindow(hwnd))
		hwnd = ::FindWindow(NULL,_T("DebugHelper "));

	if(hwnd)
		::SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szMsg);

#ifdef DEBUG_TRACE_FILE
	//将此住处信息写入文件
	static CCriticalSection cs;//支持MultiThread
	if(cs.Lock(4*1000))
	{
		try
		{
			//定义此static是为了避免反复打开文件,浪费mem and cpu
			//CFile::~CFile()会自动Close(),但不能保证程序异常时也总能调用CFile::~CFile()
			//最快的应该是File Mapping,但也不能保证异常时对文件的修改能生效.
			static CFile s_DebugTraceFile;
			if(s_DebugTraceFile.m_hFile == (HANDLE)CFile::hFileNull )
			{
				TRACE("open.............");
				CString szLogFile;
				::GetModuleFileName(NULL,szLogFile.GetBuffer(MAX_PATH+1),MAX_PATH);
				szLogFile.ReleaseBuffer();
				szLogFile += ".log";
				
				CFileStatus fs;
				if(CFile::GetStatus(szLogFile,fs))
				{
					//make sure log file size is smaller than 100 kb,otherwise empty it!
					if(fs.m_size > 100 * 1024)
						::DeleteFile(szLogFile);
				}
				s_DebugTraceFile.Open(szLogFile,CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate);
				s_DebugTraceFile.SeekToEnd();
			}
			if(s_DebugTraceFile.m_hFile != CFile::hFileNull )
			{
				CString szTime = CTime::GetCurrentTime().Format("%Y.%m.%d %H:%M:%S ");
				szMsg = szTime + szMsg + "\r\n";
				s_DebugTraceFile.Write(szMsg,szMsg.GetLength());
			}
		}
		catch(...)
		{
		}
		cs.Unlock();
	}
#endif	//end of #ifdef DEBUG_TRACE_FILE

	return TRUE;
#else	// else of #if defined __AFX_H__

	// no MFC used
	static HWND hwnd = ::FindWindow(NULL,"DebugHelper ");
	if(!IsWindow(hwnd))
		hwnd = ::FindWindow(NULL,"DebugHelper ");

	if(hwnd)
	{
		char szMsg[1000];

		va_list argList;
		va_start(argList, lpszFormat);
		try
		{
			vsprintf(szMsg,lpszFormat, argList);
		}
		catch(...)
		{
			strcpy(szMsg ,"DebugHelper输出字符串格式错误!");
		}
		va_end(argList);

		::SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szMsg);
	}

#endif	//end of #if defined __AFX_H__
	return TRUE;
}
#endif

#if defined __AFX_H__

//SfxCreatePath create a path,
//TRUE if path is exist or created successful
//FALSE is path can't be create or path isn't a directory
//Usage SfxCreatePath("d:\\asdlf\\egsdf\\ertes\\bd");
//don't set nStartPos,nStartPos is used for iterative by itself.
BOOL SfxCreatePath(CString szPath,int nStartPos/*=2*/ )
{
	ASSERT(szPath[1] == ':');//such as "c:\\sdf\\asdf\\dfbdf";
	
	if(SfxIsDirectory(szPath))
		return TRUE;

	int nPos = szPath.Find('\\',nStartPos);
	if(nPos == -1)
		return SfxIsDirectory(szPath);
	int nEndPos = szPath.Find('\\',nPos + 1);
	BOOL bReachEnd = FALSE;
	if(nEndPos == -1)
	{
		bReachEnd = TRUE;
		nEndPos = szPath.GetLength();
	}
	//int nLen = (nEndPos != -1)?(nEndPos - nPos-1):(szPath.GetLength() - nPos-1);
	CString szFolder = szPath.Left(nEndPos);
	CreateDirectory(szFolder,NULL);
	if(!PathFileExists(szFolder))
		return FALSE;
	if(bReachEnd)
		return SfxIsDirectory(szPath);
	return SfxCreatePath(szPath,nEndPos);
}				  

//SfxIsDirectory return TRUE if szPath is a directory,or FALSE
BOOL SfxIsDirectory(CString szPath)
{
	return PathIsDirectory(szPath);
}


static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED && lpData != NULL)
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	return 0;
}

//SfxBrowseFolder is used for browse a folder
//return IDOK if a folder is selected,otherwise IDCANCEL
UINT SfxBrowseFolder(CString &szFolder,LPCTSTR szDisplay,CWnd *pParentWnd,UINT uFlags)
{
	CoInitialize(NULL);

	LPMALLOC		pMalloc = NULL;
	BROWSEINFO		bi = {0};
	LPITEMIDLIST	pidl;
	int				nRetValue = IDCANCEL;

	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		
		if (pParentWnd != NULL)
			bi.hwndOwner = pParentWnd->m_hWnd;
		else if(AfxGetMainWnd())
			bi.hwndOwner = AfxGetMainWnd()->m_hWnd;

		bi.pidlRoot = NULL;
		TCHAR szBuf[MAX_PATH+1];
		szBuf[0] = _T('\0');
		bi.pszDisplayName = szBuf;
		bi.lpszTitle = szDisplay;
		bi.ulFlags = uFlags;
#ifndef _DEBUG
		bi.ulFlags |= 0x40;//new UI
#endif
		bi.lpfn = BrowseCallbackProc;
		

		if(!szFolder.IsEmpty())
			_tcsncpy(szBuf,(LPCTSTR)szFolder,MAX_PATH);

		bi.lParam = (LPARAM)szBuf;
		
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, szBuf))
			{
				szFolder = szBuf;
				nRetValue = IDOK;
			}
			pMalloc->Free(pidl);
		} 
		pMalloc->Release();
	}

	CoUninitialize();

	return nRetValue;
}

//SfxGetpathSplit return part of a full filename,such as drive,dir,filename,title,ext,ect...
CString SfxGetPathSplit(LPCTSTR szFullFileName,DWORD dwFlags)
{
	TCHAR szBuf[MAX_PATH];
	szBuf[0] = _T('\0');

	CString szRet;
	if(dwFlags == PS_EXT)
	{
		_tsplitpath(szFullFileName, NULL, NULL, NULL, szBuf);
		szRet = szBuf;
		if(!szRet.IsEmpty() && szRet[0] == _T('.'))
		{
			szRet.Remove(_T('.'));
			return szRet;
		}
	}
	else if(dwFlags == PS_DRIVE)
	{
		_tsplitpath(szFullFileName, szBuf, NULL, NULL, NULL);
		return szBuf;
	}
	else if(dwFlags == PS_TITLE)
	{
		_tsplitpath(szFullFileName, NULL,NULL,szBuf,NULL);
		return szBuf;
	}
	else if(dwFlags == PS_DIR)
	{
		TCHAR szDir[MAX_PATH];

		_tsplitpath(szFullFileName, szBuf, szDir, NULL, NULL);
		::lstrcat(szBuf, szDir);
		szRet = szBuf;
		if(!szRet.IsEmpty() && szRet.Right(1) == _T('\\'))
			szRet = szRet.Left(szRet.GetLength()-1);
		return szRet;
	}
	else if(dwFlags == PS_FILENAME)
	{
		szRet = szFullFileName;
		int nPos = szRet.ReverseFind(_T('\\'));
		if(nPos != -1)
			szRet = szRet.Right(szRet.GetLength() - nPos -1);
		return szRet;
	}

	return "";
}

CString SfxGetModulePath(HINSTANCE hInst)
{
	TCHAR szBuf[MAX_PATH];
	GetModuleFileName(hInst,szBuf,MAX_PATH);
	CString szPath(szBuf);
	szPath = SfxGetPathSplit(szPath,PS_DIR);
	return szPath;
}

void CALLBACK MessageBoxTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime)
{
	static TCHAR szBuf[MAX_PATH+1];
	g_nTimedMsgBox --;
	if(g_nTimedMsgBox <= 0)
	{
		szBuf[0] = '\0';//清空本次保存的caption
		PostQuitMessage(0);
	}
	else
	{
		CWnd *pWnd = AfxGetMainWnd();
		if(pWnd)
		{	
			pWnd = pWnd->GetLastActivePopup();
			if(pWnd)
			{
				//保存原始caption)
				CString szCaption;
				if(g_bInitCaption)
				{
					pWnd->GetWindowText(szBuf,MAX_PATH);
					g_bInitCaption = FALSE;
				}
				//TCHAR szFormat[(MAX_PATH+1)*sizeof(TCHAR)];
				//AfxLoadString(IDS_TIMED_MSGBOX_SUFFIX,szFormat,MAX_PATH);
				char *szFormat = "Left time:%d";
				//"%s (left time:%d seconds)
				szCaption.Format(szFormat,szBuf,g_nTimedMsgBox);
				pWnd->SetWindowText(szCaption);
			}
		}
		return;
	}
	PostQuitMessage(0);
}
UINT SfxTimedMessageBox(HWND hwndParent,UINT nIDMessage,UINT nIDTitle,UINT nIDDefault,
						UINT flags,DWORD dwTimeout)
{
	CString szMsg,szTitle;
	szMsg.LoadString(nIDMessage);
	szTitle.LoadString(nIDTitle);

	return SfxTimedMessageBox(hwndParent,szMsg,szTitle,nIDDefault,flags,dwTimeout);
}
UINT SfxTimedMessageBox(HWND hwndParent,LPCTSTR ptszMessage,LPCTSTR ptszTitle,UINT nIDDefault,
						UINT flags,DWORD dwTimeout)
{
   
	UINT_PTR idTimer;
	UINT uiResult;
	MSG msg;

	/*
	*  Set a timer to dismiss the message box.
	*/ 
	g_nTimedMsgBox = dwTimeout / 1000 +1;
	g_bInitCaption = TRUE;
	idTimer = ::SetTimer(NULL, 0, 1000, (TIMERPROC)MessageBoxTimer);

	if(!hwndParent)
	{
		CWnd *pwnd = AfxGetMainWnd();
		if(pwnd)
			hwndParent = pwnd->m_hWnd;
	}

	uiResult = MessageBox(hwndParent, ptszMessage, ptszTitle, flags);

	/*
	*  Finished with the timer.
	*/ 
	KillTimer(NULL, idTimer);

	/*
	*  See if there is a WM_QUIT message in the queue. If so,
	*  then you timed out. Eat the message so you don't quit the
	*  entire application.
	*/ 
	if (PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE)) {

	   /*
		*  If you timed out, then return desinated value.
		*/ 
	   uiResult = nIDDefault;
	}

	return uiResult;
}

class SFX_SHELL
{
public:
	SFX_SHELL()
	{
		if(m_hShellModule == NULL)
		{
			//don't load DLL until actual call SHAutuComplete
			m_hShellModule = LoadLibrary(_T("Shlwapi.dll"));
			if(m_hShellModule)
			{
				::CoInitialize(NULL);
				m_pSHAutoComplete = (PSHAutoComplete)GetProcAddress(
									m_hShellModule ,(LPCSTR)(_T("SHAutoComplete")));
			}
		}
	}
	~SFX_SHELL()
	{
		::CoUninitialize();
	}

	HRESULT SHAutoComplete(HWND hWndEdit,DWORD dwFlags =0)
	{
		if(m_pSHAutoComplete)
			return m_pSHAutoComplete(hWndEdit,dwFlags);
		return E_FAIL;
	}
private:
	static HMODULE m_hShellModule;
	static PSHAutoComplete m_pSHAutoComplete;
};

HMODULE SFX_SHELL::m_hShellModule = NULL;
PSHAutoComplete SFX_SHELL::m_pSHAutoComplete = NULL;

HRESULT	SfxSHAutoComplete(HWND hWnd,DWORD dwFlags)
{
	static SFX_SHELL ss;
	return ss.SHAutoComplete(hWnd,dwFlags);
}
HRESULT	SfxSHAutoComplete(CWnd *pWnd,DWORD dwFlags)
{
	if(pWnd)
		return SfxSHAutoComplete(pWnd->m_hWnd,dwFlags);
	return E_FAIL;
}


BOOL SfxChangeShellLink(LPCTSTR lpszLink,LPCTSTR lpszDest,DWORD dwFlags)
{
	return FALSE;
	/*
	LPITEMIDLIST lpII = NULL;
	HRESULT hr = ::SHGetSpecialFolderLocation(NULL,CSIDL_COMMON_PROGRAMS,&lpII);
	if(FAILED(hr))
		return FALSE;

	BOOL bOK = FALSE;
	char szFolder[MAX_PATH];
	if(::SHGetPathFromIDList(lpII, szFolder))
	{
		strcat(szFolder,lpszLink);
		
		IShellLink *pShLink = NULL;
		hr = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,
				IID_IShellLink,(LPVOID *)&pShLink);
		
		IPersistFile *ppf = NULL;
		hr = pShLink->QueryInterface(IID_IPersistFile,(LPVOID *)&ppf);

		LPCTSTR lpszLink = szFolder;
		WORD wsz[MAX_PATH*2];
		MultiByteToWideChar( CP_ACP, 0,lpszLink,-1, wsz, MAX_PATH );
		hr = ppf->Load(wsz, STGM_READ );
		if(SUCCEEDED(hr))
		{
			hr = pShLink->Resolve(NULL,SLR_ANY_MATCH | SLR_NO_UI);
			if(SUCCEEDED(hr))
			{
				pShLink->SetPath(lpszDest);
				pShLink->SetArguments("");
				hr = ppf->Save(wsz, TRUE);
				if(SUCCEEDED(hr))
					bOK = TRUE;
			}
		}
		
		ppf->Release();
		pShLink->Release();
	}
	return bOK;
	//*/
}
#endif