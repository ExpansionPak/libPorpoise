// colorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "color.h"
#include "colorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

/////////////////////////////////////////////////////////////////////////////
// CColorDlg dialog

CColorDlg::CColorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorDlg)
	DDX_Control(pDX, IDC_BUTTON_MESSAGE, m_cButtonMessage);
	DDX_Control(pDX, IDC_SLIDER_RED, m_cSliderRed);
	DDX_Control(pDX, IDC_SLIDER_GREEN, m_cSliderGreen);
	DDX_Control(pDX, IDC_SLIDER_BLUE, m_cSliderBlue);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CColorDlg, CDialog)
	//{{AFX_MSG_MAP(CColorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MESSAGE, OnButtonMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorDlg message handlers

BOOL CColorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	ItemInitialize();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CColorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CColorDlg::OnPaint() 
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
HCURSOR CColorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define CCOLOR_DLG_TIMER_ID		0x4288
#define CCOLOR_DLG_TIMER_ELAPSE	100

void CColorDlg::ItemInitialize()
{
	//	Hio Access Initialize
	if(!m_cHioAccess.Initialize()){
		CString msg;
		msg.LoadString( IDS_ERROR_INITIALIZE );
		AfxMessageBox( msg );
		PostMessage( WM_CLOSE );
	}else{
		//	Set title string
		CString title;
		title.LoadString( IDS_STRING_TITLE );
		SetWindowText( title );
		//	Slider Initialize
		m_cSliderRed.SetRange( 0, 255 );
		m_cSliderGreen.SetRange( 0, 255 );
		m_cSliderBlue.SetRange( 0, 255 );
	}
}

void CColorDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CString msg;
	switch(nSBCode){
	case SB_PAGELEFT:
	case SB_PAGERIGHT:
	case SB_LINELEFT:
	case SB_LINERIGHT:
	case SB_LEFT:
	case SB_RIGHT:{
		CSliderCtrl *pSlider = (CSliderCtrl*)pScrollBar;
		nPos = pSlider->GetPos();
	}
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:{
		CString strColor;
		UINT	nColor = nPos;
		// Set color value.
		if( pScrollBar == (CScrollBar*)(&m_cSliderRed) ){
			m_cHioAccess.SetColorRed( nColor );
			strColor="RED";
		}else if( pScrollBar == (CScrollBar*)(&m_cSliderGreen) ){
			m_cHioAccess.SetColorGreen( nColor );
			strColor="GREEN";
		}else if( pScrollBar == (CScrollBar*)(&m_cSliderBlue) ){
			m_cHioAccess.SetColorBlue( nColor );
			strColor="BLUE";
		}
		// The state is shown by the title bar. 
		msg.Format( "%s:%d" , strColor , nColor );
		SetWindowText( msg );
	}	break;
	case SB_ENDSCROLL:{
		CString title;
		title.LoadString( IDS_STRING_TITLE );
		SetWindowText( title );
	}	break;
	default:
		break;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CColorDlg::OnButtonMessage() 
{
	if(!m_cButtonMessage.GetCheck()){
		CString message;
		// set mode for no message
		if(!m_cHioAccess.ChangeMode( FALSE )){
			message.LoadString( IDS_ERROR_CHANGEMODE );
			AfxMessageBox(message);
		}
		// stop message polling timer
		KillTimer( CCOLOR_DLG_TIMER_ID );
		// message
		message.LoadString( IDS_MESSAGE_NONE );
		m_cButtonMessage.SetWindowText(message);
	}else{
		CString message;
		// set mode for request message
		if(!m_cHioAccess.ChangeMode(TRUE)){
			message.LoadString( IDS_ERROR_CHANGEMODE );
			AfxMessageBox(message);
		}
		// start message polling timer
		SetTimer( CCOLOR_DLG_TIMER_ID, CCOLOR_DLG_TIMER_ELAPSE, NULL );
	}
}

void CColorDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == CCOLOR_DLG_TIMER_ID ){
		//	polling message from target
		CString message;
		if(!m_cHioAccess.GetMessage(message)){
		}else{
			m_cButtonMessage.SetWindowText(message);
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

