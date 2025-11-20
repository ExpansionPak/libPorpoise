// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "usbsample.h"

#include "MainFrm.h"
#include "ChildView.h"
#include "Viewer.h"
#include "MultiChannel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CViewer g_Viewer;
extern CChildView*	g_pFIOView;

static HCURSOR hWait,hNormal;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_COMMAND_EX(IDD_DIALOG_FILER, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDD_DIALOG_FILER, OnUpdateControlBarMenu)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSEDIR, OnButtonChooseDir)
	ON_BN_CLICKED(IDC_CHECK_ENABLESRV, OnCheckEnablesrv)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	{
		// Initialize dialog bar m_wndMyDialogBar
		if (!m_wndMyDialogBar.Create(this, IDD_DIALOG_FILER,
			CBRS_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE,
			IDD_DIALOG_FILER))
		{
			TRACE0("Failed to create dialog bar m_wndMyDialogBar\n");
			return -1;		// fail to create
		}
		m_wndMyDialogBar.EnableDocking(CBRS_ALIGN_ANY);
		EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_wndMyDialogBar);
		m_wndMyDialogBar.SetWindowText("FIO Filer");
		UpdateListBox();
	}
	CEdit* pMask = static_cast<CEdit*>(m_wndMyDialogBar.GetDlgItem(IDC_EDIT_MASK));
	ASSERT(pMask);
	pMask->SetWindowText("*.*");

	SetWindowText("USB Communication Library Sample Program");
	hWait = AfxGetApp()->LoadStandardCursor(IDC_WAIT);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	cs.style &= ~FWS_ADDTOTITLE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnButtonRefresh() 
{
	UpdateListBox();
}

void CMainFrame::UpdateListBox()
{
	TCHAR tcTemp[MAX_PATH];
	CUsbsampleApp* pApp = static_cast<CUsbsampleApp*>(AfxGetApp());
	CString root,fiopath;
	CString fioroot = pApp->m_strFIORootDirectory;
	CEdit* pPath = static_cast<CEdit*>(m_wndMyDialogBar.GetDlgItem(IDC_EDIT_PATH));
	CEdit* pRoot = static_cast<CEdit*>(m_wndMyDialogBar.GetDlgItem(IDC_EDIT_ROOT));
	CEdit* pMask = static_cast<CEdit*>(m_wndMyDialogBar.GetDlgItem(IDC_EDIT_MASK));
	CListBox *plBox = static_cast<CListBox*>(m_wndMyDialogBar.GetDlgItem(IDC_LIST1));

	ASSERT(pPath);
	ASSERT(pRoot);
	ASSERT(pMask);
	ASSERT(plBox);

	pRoot->SetWindowText(fioroot);
	pPath->GetWindowText(fiopath);

	CString mask;
	pMask->GetWindowText(mask);
	if( mask.IsEmpty() )
		mask = "*.*";
	CString full = fioroot + "\\" + fiopath + "\\" + mask;
	lstrcpy(tcTemp,full);
	plBox->ResetContent();
	m_wndMyDialogBar.DlgDirList(tcTemp,IDC_LIST1,0,0x0010);
}

void CMainFrame::OnDblclkList1() 
{
	TCHAR tcTemp[MAX_PATH];
	CEdit* pEdit = static_cast<CEdit*>(m_wndMyDialogBar.GetDlgItem(IDC_EDIT_PATH));
	CString fiopath;
	CString sel;

	ASSERT(pEdit);

	pEdit->GetWindowText(fiopath);
	m_wndMyDialogBar.DlgDirSelect(tcTemp,IDC_LIST1);
	sel = tcTemp;
	if( sel == "..\\" ){
		int idx;
		if( fiopath.Right(1) == "\\" )
			fiopath = fiopath.Left(fiopath.GetLength()-1);
		idx = fiopath.ReverseFind('\\');
		if( idx >= 0 ){
			fiopath = fiopath.Left(idx+1);
		}
		pEdit->SetWindowText(fiopath);
		UpdateListBox();
	} else if( sel.Right(1) == "\\" ){	// directory
			if( fiopath.Right(1) == "\\" ){
				fiopath += tcTemp;
			} else {
				fiopath += CString("\\")+tcTemp;
			}
			pEdit->SetWindowText(fiopath);
			UpdateListBox();
	} else {
		CString s;
		s.Format("Filename : %s\n",fiopath+tcTemp);
		g_pFIOView->AddString(s);
		try {
			g_Viewer.SendFileName(fiopath+tcTemp);
		}
		catch(CViewerException& e){
			AfxMessageBox(e.What());
		}
	}
}

void CMainFrame::OnButtonChooseDir() 
{
	CUsbsampleApp* pApp = static_cast<CUsbsampleApp*>(AfxGetApp());
	pApp->OnAppChooseDir();
}

void CMainFrame::OnCheckEnablesrv() 
{
	CUsbsampleApp* pApp = static_cast<CUsbsampleApp*>(AfxGetApp());

	pApp->OnAppStartserver();
	SetCheck(pApp->m_bActiveServer);
}

BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	if( nHitTest == HTCLIENT ){
		if( g_Viewer.m_nLastResultStatus == -2 ){
			SetCursor(hWait);
			return TRUE;
		}
	}
	return CMDIFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CMainFrame::SetCheck(bool bCheck)
{
	CButton *pBtn = static_cast<CButton*>(m_wndMyDialogBar.GetDlgItem(IDC_CHECK_ENABLESRV));
	ASSERT(pBtn);

	pBtn->SetCheck( bCheck ? 1 : 0 );
}

