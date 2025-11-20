// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "usbsample.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	
	// Do not call CWnd::OnPaint() for painting messages
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd ::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_Font.CreatePointFont(140,"FixedSys");

	CRect rect;
	GetClientRect(rect);
	m_ctlEdit.Create(WS_CHILD | ES_AUTOVSCROLL | ES_MULTILINE,rect,this,101);
	m_ctlEdit.SetFont(&m_Font);
	m_ctlEdit.ShowScrollBar(SB_VERT);
	m_ctlEdit.ShowWindow(SW_NORMAL);
	return 0;
}

void CChildView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	CRect rect;
	GetClientRect(rect);
	m_ctlEdit.MoveWindow(rect);
}

void CChildView::AddString(LPCTSTR lpStr)
{
	INT iLen = m_ctlEdit.GetWindowTextLength();
	m_ctlEdit.SetSel(iLen,iLen);

	CString s;
	LPCTSTR p = lpStr;
	while( *p ){
		if( *p == '\r' && *(p+1) != '\n'  ){
			s += "\r\n";
			p += 2;
		} else if( *p == '\n' ){
			s += "\r\n";
			p++;
		} else {
			s += *p++;
		}
	}
	m_ctlEdit.ReplaceSel(s);
}

void CChildView::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd ::OnSetFocus(pOldWnd);
	m_ctlEdit.SetFocus();
}
