/*---------------------------------------------------------------------*

Project:  mcc viewer server - number dialog module
File:     NumberDlg.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

#if !defined(AFX_NUMBERDLG_H__779C3F21_B746_11D4_9146_0000E8DB33B8__INCLUDED_)
#define AFX_NUMBERDLG_H__779C3F21_B746_11D4_9146_0000E8DB33B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumberDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumberDlg dialog

class CNumberDlg : public CDialog
{
// Construction
public:
	CNumberDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNumberDlg)
	enum { IDD = IDD_DIALOG_NUM };
	CEdit	m_editNum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumberDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	int m_nMax;
	int m_nMin;
	int m_nNumber;

protected:

	// Generated message map functions
	//{{AFX_MSG(CNumberDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMBERDLG_H__779C3F21_B746_11D4_9146_0000E8DB33B8__INCLUDED_)
