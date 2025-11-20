// colorDlg.h : header file
//

#if !defined(AFX_COLORDLG_H__0B614CD8_EB76_11D4_9157_0000E8DB33B8__INCLUDED_)
#define AFX_COLORDLG_H__0B614CD8_EB76_11D4_9157_0000E8DB33B8__INCLUDED_

#include "HioAccess.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CColorDlg dialog

class CColorDlg : public CDialog
{
// Construction
public:
	void ItemInitialize();
	CColorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CColorDlg)
	enum { IDD = IDD_COLOR_DIALOG };
	CButton	m_cButtonMessage;
	CSliderCtrl	m_cSliderRed;
	CSliderCtrl	m_cSliderGreen;
	CSliderCtrl	m_cSliderBlue;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CHioAccess	m_cHioAccess;

	// Generated message map functions
	//{{AFX_MSG(CColorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonMessage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORDLG_H__0B614CD8_EB76_11D4_9157_0000E8DB33B8__INCLUDED_)
