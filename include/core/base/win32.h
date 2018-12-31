#pragma once
//XiongWanPing 2008~

#ifndef UNUSED
#define UNUSED(x) (void)(x)	//avoid gcc warning: unused variable
#endif


#ifdef _MSC_VER
#define SUPER(X) using X::X;
#else
#define SUPER(X) using X::X; private: typedef X __super;
#endif

#ifndef _MSC_VER
#define _open  open
#define _close close
#define _read  read
#define _write write
typedef long* LONG_PTR;
#endif

/*
#ifndef _MSC_VER
//http://blog.csdn.net/tedious/article/details/4761827
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#define CONTAINING_RECORD container_of
#endif
*/

#ifdef __APPLE__
#ifdef DEBUG		//xcode环境中debug版会定义DEBUG
	#define _DEBUG
#endif
#endif

#ifdef _MSC_VER
#define FMT_LONGLONG "%I64d"
#else
#define FMT_LONGLONG "%lld"

#define LPCTSTR  LPCSTR

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (LPBYTE)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))

#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          (P)
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)
#endif

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <queue>
#include <algorithm>

#ifndef COUNT_OF
#define COUNT_OF(x)	(int)((sizeof(x)/sizeof((x)[0])))
#endif

#ifndef _countof
#define _countof(x)	(sizeof(x)/sizeof((x)[0]))
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  do{ if(p) { delete (p);     (p)=NULL; } }while(0)
#endif

#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p)					{if(p != NULL) { delete[] (p);   (p) = NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) do{ if(p) { (p)->Release(); (p)=NULL; } }while(0)
#endif

#ifndef CheckPointer
#define CheckPointer(pointer,hr)		{if(pointer == NULL) return hr;};
#endif

//#define CriticalSection	CriticalSection

/*
#ifndef DT_MIN
#define DT_MIN	0
enum eDTLevel
{
	DT_DISABLE	=0,
	DT_FATAL,
	DT_ERROR,
	DT_WARNING,
	DT_NOTICE,
	DT_TRACE,
	DT_VERBOSE,
	DT_MAX,
};
#endif
//*/

#if defined _MSC_VER && defined _DEBUG
#define _MSC_VER_DEBUG
//#define new DEBUG_NEW
#endif

#ifdef _MSC_VER
#include <fcntl.h>
#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_RAW           _O_BINARY
#define O_TEMPORARY     _O_TEMPORARY
#define O_NOINHERIT     _O_NOINHERIT
#define O_SEQUENTIAL    _O_SEQUENTIAL
#define O_RANDOM        _O_RANDOM

#define O_NONBLOCK		0

#endif

#ifdef _MSC_VER
	#ifndef __func__
	#define __func__ __FUNCTION__
	#endif

#ifndef __PRETTY_FUNCTION__
	#define __PRETTY_FUNCTION__	__func__
#endif

#define ssize_t	SSIZE_T
#define PACKED
#else
#define PACKED	__attribute__((packed))
#define _stat	stat
#define _chdir	chdir
#define _stricmp strcasecmp

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


#ifndef INVALID_SOCKET
#define INVALID_SOCKET	(-1)
#endif

#ifndef COUNT_OF
#define COUNT_OF(x)	(sizeof(x)/sizeof((x)[0]))
#endif

//typedef unsigned long HANDLE;
typedef int HANDLE;

#if defined __APPLE__
	#if __LP64__
	typedef bool BOOL;
	#else
	typedef signed char BOOL; //ipad air compile error
	#endif
#else
	typedef int BOOL;
#endif


typedef unsigned int DWORD,*LPDWORD;
typedef unsigned long *WPARAM;
typedef unsigned long *LPARAM;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;
typedef long long	LONGLONG; //64 bit
typedef unsigned long long	ULONGLONG; //64 bit
typedef unsigned long long	ULONGULONG;
typedef unsigned short WORD;
typedef WORD *LPWORD;
typedef void VOID;
typedef VOID *	LPVOID;
typedef VOID *	PVOID;
typedef long LRESULT;
typedef unsigned char BYTE,*LPBYTE;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef const char * LPCSTR,*LPCSTR;
typedef char TCHAR, *PTCHAR,*LPTSTR;

#ifdef _MSC_VER
typedef __int64		INT64;
#else
typedef long long	INT64;
#define CALLBACK    __stdcall
#define WINAPI      //__stdcall
#define WINAPIV     __cdecl
#define APIENTRY    WINAPI
#define APIPRIVATE  __stdcall
#define PASCAL      __stdcall
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))
#define MAKEWPARAM(low,high)		(LRESULT) ( (low&0xFFFF) + ((high&0xFFFF)<<16) )
#define MAKELPARAM(low,high)		(LRESULT) ( (low&0xFFFF) + ((high&0xFFFF)<<16) )

typedef int SOCKET;
//#define SD_BOTH			SHUT_RDWR
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

#define SOCKET_ERROR	-1
#define closesocket close
#define _snprintf snprintf
#define _T

#define _strnicmp strncasecmp

#define FAR
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;

#define IN
#define OUT
#define INOUT

#ifndef MAX_PATH
#define MAX_PATH	266
#endif

#ifndef NULL
#define NULL	0
#endif

namespace Bear {
namespace Core
{
typedef struct tagSIZE
{
	LONG        cx;
	LONG        cy;
} SIZE, *PSIZE, *LPSIZE;

class CORE_EXPORT CSize :public tagSIZE
{
public:
	CSize()
	{
		cx = 0;
		cy = 0;
	}

	BOOL operator!=(SIZE size) const
	{
		return (cx != size.cx || cy != size.cy);
	}

	BOOL operator==(SIZE size) const
	{
		return (cx == size.cx && cy == size.cy);
	}

	CSize(long initCX, long initCY)
	{
		cx = initCX;
		cy = initCY;
	}
};

#undef FAR
#undef  NEAR
#define FAR                 far
#define NEAR                near
#ifndef CONST
#define CONST               const
#endif

typedef struct tagPOINT
{
	LONG  x;
	LONG  y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

class CORE_EXPORT CPoint :public tagPOINT
{
public:
	CPoint()
	{
		x = y = 0;
	}
	CPoint(LONG xx, LONG yy)
	{
		x = xx;
		y = yy;
	}

	//XiongWanPing 2011.10.13
	//不重载的话,mips上面CPoint pt=CPoint(0,0)会有奇怪问题
	CPoint& operator=(const CPoint& pt)
	{
		x = pt.x;
		y = pt.y;
		return *this;
	}
	BOOL operator!=(const CPoint& pt)
	{
		return x != pt.x || y != pt.y;
	}
	BOOL operator==(const CPoint& pt)
	{
		return x == pt.x && y == pt.y;
	}

	void Offset(int dx, int dy)
	{
		x += dx;
		y += dy;
	}
};

typedef struct tagRECT
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

class CORE_EXPORT CRect : public tagRECT
{
	// Constructors
public:
	CRect()
	{
		left = top = right = bottom = 0;
	}
	CRect(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}
	CRect(const RECT& srcRect)
	{
		left = srcRect.left;
		top = srcRect.top;
		right = srcRect.right;
		bottom = srcRect.bottom;
	}
	CRect(POINT point, SIZE size)
	{
		left = point.x;
		top = point.y;
		right = left + size.cx;
		bottom = top + size.cy;
	}
	// from two points
	CRect(POINT topLeft, POINT bottomRight)
	{
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}

	long Width()
	{
		return right - left;
	}
	// returns the height
	long Height()
	{
		return bottom - top;
	}

	CSize Size()
	{
		return CSize(right - left, bottom - top);
	}
	CPoint TopLeft()
	{
		return CPoint(left, top);
	}
	const CPoint BottomRight()
	{
		return CPoint(right, bottom);
	}
	operator LPRECT()
	{
		return this;
	}

	// returns TRUE if rectangle has no area
	BOOL IsRectEmpty()
	{
		return left == right || top == bottom;
	}
	void SetRect(int x1, int y1, int x2, int y2)
	{
		left = x1;
		top = y1;
		right = x2;
		bottom = y2;
	}
	void SetRect(POINT topLeft, POINT bottomRight)
	{
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}
	void SetRectEmpty()
	{
		left = top = right = bottom = 0;
	}
	void NormalizeRect()
	{
		if (left > right)
		{
			long x = right;
			right = left;
			left = x;
		}
		if (top > bottom)
		{
			long x = bottom;
			bottom = top;
			top = x;
		}
	}
};
}
}

//typedef DWORD   COLORREF;//和hi3516中的定义有冲突,所以屏蔽了
//#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __inline
#elif __GNUC__
#define INLINE __inline__
#else
#define INLINE
#endif
#endif

/*
void DebugTrace(int nLevel,const char *pszFile,int nLine,const char * lpszFormat,...);

int  SetDTLevel(int level);
int  GetDTLevel(void);

#if defined _DEBUG || defined _ENABLE_DT
	#define DV(lpszFormat,args...)	do{ DebugTrace(DT_VERBOSE,__FILE__,__LINE__,lpszFormat,##args);}while(0)
	#define DT(lpszFormat,args...)	do{ DebugTrace(DT_TRACE,__FILE__,__LINE__,lpszFormat,##args);}while(0)
	#define DW(lpszFormat,args...)	do{ DebugTrace(DT_WARNING,__FILE__,__LINE__,lpszFormat,##args);}while(0)
	#define DE(lpszFormat,args...)	do{ DebugTrace(DT_ERROR,__FILE__,__LINE__,lpszFormat,##args);}while(0)
#else
	#define DV
	#define DT
	#define DW
	#define DE
#endif

#define DF(lpszFormat,args...)	do{ DebugTrace(DT_FATAL,__FILE__,__LINE__,lpszFormat,##args);}while(0)
//*/
#endif

#ifndef MIN
#define	MIN(a, b)		(((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a, b)		(((a)>(b))?(a):(b))
#endif

#ifndef CLR_BUF
// !=sizeof(void*)是为了防止误用指针
#define CLR_BUF(x)	do{memset(x,0,sizeof(x));}while(0)
#endif

#ifdef _MSC_VER
typedef int socklen_t;
#define bzero(pBuf,cbSize) do {memset(pBuf,0,cbSize);}while(0)
#else
#include <strings.h>
#endif

#ifndef INFINITE
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif

#if defined __APPLE__ || defined __ANDROID__
#define THREAD_DECLARE LPVOID 
#define THREAD_IMPLEMENT LPVOID 
#else
#define THREAD_DECLARE DWORD WINAPI 
#define THREAD_IMPLEMENT DWORD 
#endif

