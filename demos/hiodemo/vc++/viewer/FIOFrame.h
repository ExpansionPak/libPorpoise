#if !defined(AFX_FIOFRAME_H__0296C7E1_F1CC_4EA8_96D8_97057426A18C__INCLUDED_)
#define AFX_FIOFRAME_H__0296C7E1_F1CC_4EA8_96D8_97057426A18C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FIOFrame.h : header file
//
#include "ChildFrm.h"
/////////////////////////////////////////////////////////////////////////////
// CFIOFrame window

class CFIOFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CFIOFrame)
// Construction
public:
	CFIOFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFIOFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFIOFrame();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFIOFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIOFRAME_H__0296C7E1_F1CC_4EA8_96D8_97057426A18C__INCLUDED_)
