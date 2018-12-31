// DebugHelperDoc.h : interface of the CDebugHelperDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUGHELPERDOC_H__75181DF8_B517_48C0_B2D9_9F11FED9FA10__INCLUDED_)
#define AFX_DEBUGHELPERDOC_H__75181DF8_B517_48C0_B2D9_9F11FED9FA10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CDebugHelperDoc : public CDocument
{
protected: // create from serialization only
	CDebugHelperDoc();
	DECLARE_DYNCREATE(CDebugHelperDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugHelperDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDebugHelperDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDebugHelperDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGHELPERDOC_H__75181DF8_B517_48C0_B2D9_9F11FED9FA10__INCLUDED_)
