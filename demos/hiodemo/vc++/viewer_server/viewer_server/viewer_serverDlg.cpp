/*---------------------------------------------------------------------*

Project:  mcc viewer server - Viewer server main dialog
File:     viewerServerDlg.cpp

(C) 2000 Nintendo

-----------------------------------------------------------------------*/
// viewer_serverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "viewer_server.h"
#include "viewer_serverDlg.h"

#include "Win32/ttyserver.h"
#include "Win32/fioserver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewerServerDlg dialog

CViewerServerDlg::CViewerServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewerServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewerServerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CViewerServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewerServerDlg)
	DDX_Control(pDX, IDC_LIST_CONFIG, m_listConfig);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnSTART);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CViewerServerDlg, CDialog)
	//{{AFX_MSG_MAP(CViewerServerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_LBN_DBLCLK(IDC_LIST_CONFIG, OnDblclkListConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewerServerDlg message handlers
static CViewerServerDlg*	cViewerServerDlg=NULL;

BOOL CViewerServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Initialize items
	InitializeParameterAndItems();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CViewerServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CViewerServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//********************//
/////////////////////////////////////////////////////////////////////////////
// Viewer Server implementation

// Dialog item event
void CViewerServerDlg::OnClose() 
{
	if(m_bServerIsRunning){
		// If the server is running,stop.
		ServerStop();
	}

	CDialog::OnClose();
}

void CViewerServerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// Relocation of item according to size change in dialog.
	RelocateItems(cx,cy);
}

void CViewerServerDlg::OnButtonStart() 
{
	if(!m_bServerIsRunning){
		// server starting process
		ServerStart();
	}else{
		// server stoping process
		ServerStop();
	}
}

void CViewerServerDlg::OnDblclkListConfig() 
{
	// Edit of congif item.
	EditConfigItem();
}


// MCC callbacks
// ------------------------------
#include <dolphin/mcc.h>

s32 m_exiChannel = -1;
BOOL WINAPI hioEnumCallback( s32 chan )
{	m_exiChannel=chan;
	return FALSE;//The device found first is used.
}

BOOL m_bMccInitialized=FALSE;
void WINAPI mccSystemCallback( MCCSysEvent event )
{
	if(event == MCC_SYSEVENT_INITIALIZED){
		m_bMccInitialized = TRUE;
		cViewerServerDlg->m_btnSTART.EnableWindow( TRUE );
	}
}

void WINAPI ttyServerCallback( char* string )
{	if(cViewerServerDlg->m_cConsole)
		cViewerServerDlg->m_cConsole->Report( string );
}

// Initializing parameter and items
// ------------------------------
void CViewerServerDlg::InitializeParameterAndItems()
{	CString errorMsg;

	// Initialize internal parameters.
	// ------------------------------
	char pathName[MAX_PATH];
	GetCurrentDirectory( MAX_PATH, pathName );
	m_strFioPath	= pathName;	// Rootpath for refered by FIO.
	m_nTtyChannel	= 8;		// MCC Channel number used by TTY
	m_nFioChannel	= 1;		// MCC Channel number used by FIO
	m_nFioBlocks	= 10;		// MCC Channel block size used by FIO
	m_bServerIsRunning = FALSE;
	m_cConsole		= NULL;
	cViewerServerDlg= this;
	// Relocate dialog items.
	// ------------------------------
	CRect wndRect;
	GetWindowRect(wndRect);
	MoveWindow(0,0,320,wndRect.Height());
	RedrawConfigItem();
	// Set caption of button item.
	// ------------------------------
	CString caption;
	caption.LoadString(IDS_STRING_START);
	m_btnSTART.SetWindowText( caption );
	m_btnSTART.EnableWindow( FALSE );

	// Initialize for MCC
	// ------------------------------
	// Detection of usb adapter
	if(!MCCEnumDevices( hioEnumCallback )){
		// Can't detect USB Adapter
		errorMsg.LoadString(IDS_STRING_ERROR_CANT_DETECT_ADAPTER);
	}else{
		if(m_exiChannel<0){
			// Please connect USB cable
			errorMsg.LoadString(IDS_STRING_ERROR_NO_ADAPTER);
		}else{
			// Initialize MCC
			if(!MCCInit((MCCExiChannel)m_exiChannel,0,mccSystemCallback)){
				// Can't initialize MCC
				errorMsg.LoadString(IDS_STRING_ERROR_CANT_INIT_MCC);
			}else{
				return;
			}
		}
	}
	AfxMessageBox( errorMsg );

	// Failed initialization
	// ------------------------------
	int  error = MCCGetLastError();
	m_btnSTART.EnableWindow( FALSE );
}

// Server processing
// ------------------------------
void CViewerServerDlg::ServerStart()
{	CString errorMsg;
	// Start server
	if(!TTYServerStart( (MCCChannel)m_nTtyChannel, (TTYServerCallback)ttyServerCallback )){
		//	Can't start TTY Server.
		errorMsg.LoadString(IDS_STRING_ERROR_CANT_START_TTY_SERVER);
		AfxMessageBox( errorMsg );
	}else{

		if(!FIOServerStart( (MCCChannel)m_nFioChannel, (u8)m_nFioBlocks, m_strFioPath.GetBuffer(MAX_PATH) )){
			//	Can't start FIO Server.
			errorMsg.LoadString(IDS_STRING_ERROR_CANT_START_FIO_SERVER);
			AfxMessageBox( errorMsg );
			TTYServerStop();
		}else{
			// Open console for TTY
			m_cConsole = new CConsole();
			if(!m_cConsole){
				//	Can't open console.
				errorMsg.LoadString(IDS_STRING_ERROR_CANT_OPEN_CONSOLE);
				AfxMessageBox( errorMsg );
				TTYServerStop();
				FIOServerStop();
			}else{
				CString caption;
				// Set caption of button item
				caption.LoadString(IDS_STRING_STOP);
				m_btnSTART.SetWindowText( caption );
				m_bServerIsRunning=!m_bServerIsRunning;
				// Set text to console
				m_cConsole->Report("Start viewer server...\n");
			}
		}
	}
}

void CViewerServerDlg::ServerStop()
{
	CString caption;
	// Stop the server.
	TTYServerStop();
	FIOServerStop();
	// Close the console.
	delete m_cConsole;
	m_cConsole=NULL;
	// Set caption of button item
	caption.LoadString(IDS_STRING_START);
	m_btnSTART.SetWindowText( caption );
	m_bServerIsRunning=!m_bServerIsRunning;
}


// Dialog items
// ------------------------------

// Redraw configuration item
void CViewerServerDlg::RedrawConfigItem()
{	CString	str;
	int index=m_listConfig.GetCurSel();

	m_listConfig.ResetContent();
	str.Format("TTY Channel :\t %d",m_nTtyChannel);
	m_listConfig.AddString(str);
	str.Format("FIO Channel :\t %d",m_nFioChannel);
	m_listConfig.AddString(str);
	str.Format("FIO Blocks :\t %d",m_nFioBlocks);
	m_listConfig.AddString(str);
	str.Format("FIO Path :\t\t %s",m_strFioPath);
	m_listConfig.AddString(str);
	//
	m_listConfig.SetCurSel(index);
}

// Edit configure item
#include "NumberDlg.h"
void CViewerServerDlg::EditConfigItem()
{
	int curValue,curSel = m_listConfig.GetCurSel();

	if(curSel<3){
		// Get value from selected item
		curValue =	(curSel==0)?(m_nTtyChannel):(
					(curSel==1)?(m_nFioChannel):(
					(curSel==2)?(m_nFioBlocks):(0)));
		CNumberDlg dlg;
		dlg.m_nMin = 1;
		dlg.m_nMax = 15;
		dlg.m_nNumber = curValue;
		if(dlg.DoModal()!=IDOK){
		}else{
			// Set input value
			switch(curSel){
			case 0:{ m_nTtyChannel=dlg.m_nNumber; }break;
			case 1:{ m_nFioChannel=dlg.m_nNumber; }break;
			case 2:{ m_nFioBlocks=dlg.m_nNumber; }break;
			}
		}
	}else{
		SelectFolder( m_strFioPath );
	}
	// Redraw item
	RedrawConfigItem();
}

//	Relocating items
void CViewerServerDlg::RelocateItems(int cx,int cy)
{
	if(!IsWindow(m_listConfig.GetSafeHwnd())){
	}else{
		CRect	btnRect, listRect;
		int		btnHeight,listHeight,frameSize=1;
		// Get height of default button item.
		m_btnSTART.GetWindowRect( btnRect );
		btnHeight = btnRect.Height(); 
		// Relocate list item
		listHeight = cy-(frameSize+btnHeight);
		listRect.SetRect(frameSize,frameSize,cx-frameSize,listHeight);
		m_listConfig.MoveWindow( listRect );
		// Relocate button item
		m_btnSTART.MoveWindow( CRect(frameSize,listHeight,cx-frameSize,listHeight+btnHeight) );
	}
}


// Select the base path directory
// ------------------------------

// The callback of SHBrowseForFolder function.
static int CALLBACK BrowseCallbackProc(
   HWND hwnd,
   UINT uMsg,
   LPARAM lParam,
   LPARAM lpData )
{
	switch( uMsg ){
		case BFFM_INITIALIZED:{
			if( LPCTSTR(lpData)[0] )
				SendMessage(hwnd,BFFM_SETSELECTION,TRUE,LPARAM(lpData));
		}	break;
	}
	return 0;
}

void CViewerServerDlg::SelectFolder( CString& pathName )
{
	BROWSEINFO bi;
	LPCITEMIDLIST pItem;
	TCHAR Buffer[MAX_PATH],Path[MAX_PATH];

	strcpy(Path,pathName);

	bi.hwndOwner = AfxGetMainWnd()->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer;
	bi.lpszTitle = ""; // title string
	bi.ulFlags = BIF_EDITBOX; // flag of dialogs
	bi.lpfn = BrowseCallbackProc; // if no need callback,set NULL.
	bi.iImage = 0;
	bi.lParam = (LPARAM)Path; // if no need argment for callback,set NULL
	if( pItem = SHBrowseForFolder( &bi ) ) {
		TCHAR Path[MAX_PATH];
		if( SHGetPathFromIDList(pItem,Path) ){//Get path name.
			pathName = Path;//printf("path=%s\n",Path);
		}
		CoTaskMemFree((void*)pItem); // free memory 
	}
}



