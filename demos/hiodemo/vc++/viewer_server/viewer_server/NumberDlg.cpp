// NumberDlg.cpp : implementation file
//

#include "stdafx.h"
#include "viewer_server.h"
#include "NumberDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumberDlg dialog


CNumberDlg::CNumberDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNumberDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNumberDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// Initialize parameter
	m_nMin = m_nMax = m_nNumber = 0;
}


void CNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNumberDlg)
	DDX_Control(pDX, IDC_EDIT_NUM, m_editNum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNumberDlg, CDialog)
	//{{AFX_MSG_MAP(CNumberDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumberDlg message handlers

BOOL CNumberDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	// Initialize dialog items
	CString numStr;
	numStr.Format("%d",m_nNumber);
	m_editNum.SetWindowText(numStr);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNumberDlg::OnOK() 
{
	CString numStr;
	int	num;
	m_editNum.GetWindowText(numStr);
	sscanf(numStr,"%d",&num);

	// Validity within the range of the value is inspected.
	// ================================
	if(num<m_nMin || num>m_nMax){
		CString msg;
		msg.Format(
			"The value is out of ramge(%d - %d)",
			m_nMin,m_nMax);
		AfxMessageBox(msg);
	}else{
		m_nNumber = num;
		CDialog::OnOK();
	}
	
	CDialog::OnOK();
}
