#if !defined(AFX_FIOVIEW_H__137355B8_9AAC_4FC3_9DD2_344D694338DB__INCLUDED_)
#define AFX_FIOVIEW_H__137355B8_9AAC_4FC3_9DD2_344D694338DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FIOView.h : header file
//
#include "ChildView.h"
/////////////////////////////////////////////////////////////////////////////
// CFIOView window

class CFIOView : public CChildView
{
// Construction
public:
	CFIOView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFIOView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFIOView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFIOView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIOVIEW_H__137355B8_9AAC_4FC3_9DD2_344D694338DB__INCLUDED_)
