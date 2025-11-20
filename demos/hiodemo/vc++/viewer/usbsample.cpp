// usbsample.cpp : Defines the class behaviors for the application.
v//

#include "stdafx.h"
#include "usbsample.h"

#include "MainFrm.h"
#include "TTYFrame.h"
#include "FIOFrame.h"

#include "Dolphin/mcc.h"
#include "Win32/FIOServer.h"
#include "Win32/TTYServer.h"
#include "MultiChannel.h"
#include "Viewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	TTY_CHANNEL	MCCChannel(1)
#define	FIO_CHANNEL	MCCChannel(2)
#define	FIO_BLOCKS	4
#define	VIEWER_CHANNEL	MCCChannel(3)
#define	VIEWER_BLOCKS	1

CMultiChannel	g_Mcc;
CChildView*	g_pTTYView;
CChildView*	g_pFIOView;
CViewer	g_Viewer(VIEWER_CHANNEL,VIEWER_BLOCKS);

static bool volatile bMccInitialized = false;
static bool volatile bMccReboot = false;

// MCC Callback
static void WINAPI mccSysCallback( MCCSysEvent event )
{
	if( event == MCC_SYSEVENT_INITIALIZED ){
		bMccInitialized = true;
	} else if( event == MCC_SYSEVENT_REBOOT ){
		bMccReboot = true;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CUsbsampleApp

BEGIN_MESSAGE_MAP(CUsbsampleApp, CWinApp)
	//{{AFX_MSG_MAP(CUsbsampleApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_STARTSERVER, OnAppStartserver)
	ON_UPDATE_COMMAND_UI(ID_APP_STARTSERVER, OnUpdateAppStartserver)
	ON_COMMAND(ID_APP_FIOROOTDIR, OnAppChooseDir)
	ON_UPDATE_COMMAND_UI(ID_APP_FIOROOTDIR, OnUpdateAppChooseDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUsbsampleApp construction

CUsbsampleApp::CUsbsampleApp()
{
	m_bActiveServer = false;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUsbsampleApp object

CUsbsampleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUsbsampleApp initialization

BOOL CUsbsampleApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	TCHAR tcPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,tcPath);
	m_strFIORootDirectory = tcPath;

	try {
		g_Mcc.Init(mccSysCallback);
	}
	catch( CMCCException& e ){
		if( e.What() )
			AfxMessageBox(e.What());
		return FALSE;
	}


	// Change the registry key under which our settings are stored.
//	SetRegistryKey(_T("Local AppWizard-Generated Applications"));



	CMDIFrameWnd* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create main MDI frame window
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	// try to load shared MDI menus and accelerator table

	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_USBSAMTYPE));
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_USBSAMTYPE));


	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();

	CTTYFrame* pTTYFrame;
	CFIOFrame* pFIOFrame;

	// create a new MDI child window
	pTTYFrame = STATIC_DOWNCAST(CTTYFrame,pFrame->CreateNewChild(
				RUNTIME_CLASS(CTTYFrame), IDR_USBSAMTYPE, m_hMDIMenu, m_hMDIAccel));
	pFIOFrame = STATIC_DOWNCAST(CFIOFrame,pFrame->CreateNewChild(
				RUNTIME_CLASS(CFIOFrame), IDR_USBSAMTYPE, m_hMDIMenu, m_hMDIAccel));
	g_pTTYView = &pTTYFrame->m_wndView;
	g_pFIOView = &pFIOFrame->m_wndView;
	pFrame->MDITile(MDITILE_HORIZONTAL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CUsbsampleApp message handlers

int CUsbsampleApp::ExitInstance() 
{
	if (m_hMDIMenu != NULL)
		FreeResource(m_hMDIMenu);
	if (m_hMDIAccel != NULL)
		FreeResource(m_hMDIAccel);

	return CWinApp::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CUsbsampleApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CUsbsampleApp message handlers

void WINAPI TTYCallback( char* str )
{
	if( g_pTTYView )
		g_pTTYView->AddString(str);
}

void CUsbsampleApp::StartServer()
{
	m_bActiveServer = false;
	if( !FIOServerStart( FIO_CHANNEL,FIO_BLOCKS, (char*)LPCTSTR(m_strFIORootDirectory) ) )
		throw CMCCException("FIO Server error");
	g_pFIOView->AddString(CString("started. ROOT=[")+m_strFIORootDirectory+"]\n");
	if( !TTYServerStart( TTY_CHANNEL,TTYCallback ) )
		throw CMCCException("TTY Server error");
	g_Viewer.Start();
	CMainFrame *pFrame = STATIC_DOWNCAST(CMainFrame,m_pMainWnd);
	pFrame->UpdateListBox();
	pFrame->SetCheck();
	m_bActiveServer = true;
}

void CUsbsampleApp::StopServer()
{
	if( m_bActiveServer ){
		FIOServerStop();
		g_pFIOView->AddString("stopped.\n");
		TTYServerStop();
		g_Viewer.Stop();
		g_Mcc.Terminate();
		m_bActiveServer = false;
		STATIC_DOWNCAST(CMainFrame,m_pMainWnd)->SetCheck(false);
	}
}

void CUsbsampleApp::OnAppStartserver() 
{
	try {
		StopServer();
		StartServer();
	}
	catch( CMCCException& e ){
		if( e.What() )
			AfxMessageBox(e.What());
	}
	catch( CViewerException& e ){
		AfxMessageBox(e.What());
	}
}

void CUsbsampleApp::OnUpdateAppStartserver(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bActiveServer ?  1 : 0);
}

// Callback for SHBrowseForFolder
static int CALLBACK BrowseCallbackProc(
									   HWND hwnd,
									   UINT uMsg, 
									   LPARAM lParam, 
									   LPARAM lpData
									   )
{
	switch( uMsg ){
		case BFFM_INITIALIZED:	// Initialized
			{
				if( LPCTSTR(lpData)[0] )	// Selection current directory
					SendMessage(hwnd,BFFM_SETSELECTION,TRUE,LPARAM(lpData));
			}
			break;
	}
	return 0;
}

void CUsbsampleApp::OnAppChooseDir() 
{
    BROWSEINFO bi;
    LPCITEMIDLIST pItem;
    TCHAR Buffer[MAX_PATH],Path[MAX_PATH];;
	CString sPath;

	bi.hwndOwner = AfxGetMainWnd()->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer;
	bi.lpszTitle = "FIO Root directory";
	bi.ulFlags = BIF_EDITBOX;
	bi.lpfn = BrowseCallbackProc;
	bi.iImage = 0;
	bi.lParam = (LPARAM)Path;
	sPath = m_strFIORootDirectory;
	if( sPath.Right(1) == "\\" )
		sPath = sPath.Left(sPath.GetLength()-1);
	lstrcpy(Path,LPCTSTR(sPath));
	if( pItem = SHBrowseForFolder( &bi ) ) {
		TCHAR Path[MAX_PATH];
		if( SHGetPathFromIDList(pItem,Path) ){
			try {
				m_strFIORootDirectory = Path;
				if( m_bActiveServer ){
					FIOServerStop();
					if( !FIOServerStart( FIO_CHANNEL,FIO_BLOCKS, (char*)LPCTSTR(m_strFIORootDirectory) ) )
						throw CMCCException("FIO Server error");
				}
				CString s;
				s.Format("Change FIO root directory : %s\n",m_strFIORootDirectory);
				g_pFIOView->AddString(s);
				CMainFrame *pFrame = STATIC_DOWNCAST(CMainFrame,m_pMainWnd);
				pFrame->UpdateListBox();
			}
			catch( CMCCException& e ){
				AfxMessageBox(e.What());
			}
		}
		CoTaskMemFree((void*)pItem);
	}
}

void CUsbsampleApp::OnUpdateAppChooseDir(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bActiveServer ?  1 : 0);
}

