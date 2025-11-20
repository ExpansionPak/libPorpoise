#if !defined(AFX_TTYVIEW_H__BB508E51_29F8_46A5_9F4D_93BB29BD3DD3__INCLUDED_)
#define AFX_TTYVIEW_H__BB508E51_29F8_46A5_9F4D_93BB29BD3DD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TTYView.h : header file
//
#include "ChildView.h"

/////////////////////////////////////////////////////////////////////////////
// CTTYView window

class CTTYView : public CChildView
{
// Construction
public:
	CTTYView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTTYView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTTYView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTTYView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TTYVIEW_H__BB508E51_29F8_46A5_9F4D_93BB29BD3DD3__INCLUDED_)
