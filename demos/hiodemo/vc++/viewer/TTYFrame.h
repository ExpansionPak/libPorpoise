#if !defined(AFX_TTYFRAME_H__ED85EE6B_C143_44AA_86F6_4D3E46228894__INCLUDED_)
#define AFX_TTYFRAME_H__ED85EE6B_C143_44AA_86F6_4D3E46228894__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TTYFrame.h : header file
//
#include "ChildFrm.h"
/////////////////////////////////////////////////////////////////////////////
// CTTYFrame window

class CTTYFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CTTYFrame)
// Construction
public:
	CTTYFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTTYFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTTYFrame();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTTYFrame)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TTYFRAME_H__ED85EE6B_C143_44AA_86F6_4D3E46228894__INCLUDED_)
