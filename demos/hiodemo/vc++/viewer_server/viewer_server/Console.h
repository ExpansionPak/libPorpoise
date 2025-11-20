/*---------------------------------------------------------------------*

Project:  mcc viewer server - consile module
File:     Console.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// Console.h: CConsole class interface
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONSOLE_H__C26AE842_AECC_11D4_8571_0090CC011405__INCLUDED_)
#define AFX_CONSOLE_H__C26AE842_AECC_11D4_8571_0090CC011405__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConsole  
{
public:
	CConsole();
	virtual ~CConsole();
	void Report( char*msg );

protected:
	HANDLE m_hConsoleStdout;
	HANDLE m_hConsoleStdin;
};

#endif // !defined(AFX_CONSOLE_H__C26AE842_AECC_11D4_8571_0090CC011405__INCLUDED_)
