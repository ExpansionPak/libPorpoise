// TTYFrame.cpp : implementation file
//

#include "stdafx.h"
#include "usbsample.h"
#include "TTYFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTTYFrame, CChildFrame)
/////////////////////////////////////////////////////////////////////////////
// CTTYFrame

CTTYFrame::CTTYFrame()
{
	m_strTitle = _T("TTY Window");
}

CTTYFrame::~CTTYFrame()
{
}


BEGIN_MESSAGE_MAP(CTTYFrame, CChildFrame)
	//{{AFX_MSG_MAP(CTTYFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTTYFrame message handlers
