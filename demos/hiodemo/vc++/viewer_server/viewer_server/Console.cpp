/*---------------------------------------------------------------------*

Project:  mcc viewer server - consile module
File:     Console.cpp

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// Console.cpp: CConsole implementation file
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "viewer_server.h"
#include "Console.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CConsole
//////////////////////////////////////////////////////////////////////

CConsole::CConsole()
{
	// CLEAR
	m_hConsoleStdin	 = NULL;
	m_hConsoleStdout = NULL;
	// OPEN CONSOLE
	if(!AllocConsole()){
	}else{
		m_hConsoleStdin	 = GetStdHandle(STD_INPUT_HANDLE);
		m_hConsoleStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		if(m_hConsoleStdin == m_hConsoleStdout){
		}else{
			return;
		}
	}
	// ERROR
	m_hConsoleStdin	 = NULL;
	m_hConsoleStdout = NULL;
}

CConsole::~CConsole()
{
	if(m_hConsoleStdout)
		FreeConsole();
	// CLEAR
	m_hConsoleStdin	 = NULL;
	m_hConsoleStdout = NULL;
}

void CConsole::Report(char *msg)
{
	if(msg){
		DWORD n;
		WriteFile(m_hConsoleStdout,msg,strlen(msg),&n,0);
	}
}
