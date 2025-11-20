/*---------------------------------------------------------------------*

Project:  mcc viewer server - Viewer server main dialog
File:     viewerServerDlg.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/
// viewer_serverDlg.h : header file
//

#if !defined(AFX_VIEWER_SERVERDLG_H__5C4FD4BC_B742_11D4_9146_0000E8DB33B8__INCLUDED_)
#define AFX_VIEWER_SERVERDLG_H__5C4FD4BC_B742_11D4_9146_0000E8DB33B8__INCLUDED_

#include "console.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CViewerServerDlg dialog

class CViewerServerDlg : public CDialog
{
// Construction
public:
	CViewerServerDlg(CWnd* pParent = NULL);	// standard constructor

	void	EditConfigItem();
	void	RedrawConfigItem();
	void	RelocateItems(int cx, int cy);
	void	SelectFolder(CString& pathName);

// Dialog Data
	//{{AFX_DATA(CViewerServerDlg)
	enum { IDD = IDD_VIEWER_SERVER_DIALOG };
	CListBox	m_listConfig;
	CButton	m_btnSTART;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewerServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	int			m_nTtyChannel;
	int			m_nFioChannel;
	int			m_nFioBlocks;
	CString		m_strFioPath;
	CConsole	*m_cConsole;
	void InitializeParameterAndItems();
	void ServerStart();
	void ServerStop();

protected:
	HICON m_hIcon;
	BOOL  m_bServerIsRunning;

	// Generated message map functions
	//{{AFX_MSG(CViewerServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnButtonStart();
	afx_msg void OnDblclkListConfig();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWER_SERVERDLG_H__5C4FD4BC_B742_11D4_9146_0000E8DB33B8__INCLUDED_)
