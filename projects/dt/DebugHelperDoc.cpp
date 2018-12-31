// DebugHelperDoc.cpp : implementation of the CDebugHelperDoc class
//

#include "stdafx.h"
#include "DebugHelper.h"

#include "DebugHelperDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperDoc

IMPLEMENT_DYNCREATE(CDebugHelperDoc, CDocument)

BEGIN_MESSAGE_MAP(CDebugHelperDoc, CDocument)
	//{{AFX_MSG_MAP(CDebugHelperDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperDoc construction/destruction

CDebugHelperDoc::CDebugHelperDoc()
{
	// TODO: add one-time construction code here

}

CDebugHelperDoc::~CDebugHelperDoc()
{
}

BOOL CDebugHelperDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	((CEditView*)m_viewList.GetHead())->SetWindowText(NULL);

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDebugHelperDoc serialization

void CDebugHelperDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperDoc diagnostics

#ifdef _DEBUG
void CDebugHelperDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDebugHelperDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperDoc commands
