// HioAccess.h: header file
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIOACCESS_H__A33EE737_EB7A_11D4_85C5_0090CC011405__INCLUDED_)
#define AFX_HIOACCESS_H__A33EE737_EB7A_11D4_85C5_0090CC011405__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CHioAccess  
{
public:
	BOOL ClearMemory();
	int m_nExiChan;

	CHioAccess();
	virtual ~CHioAccess();

	BOOL Initialize();
	BOOL SetColorRed( BYTE value );
	BOOL SetColorGreen( BYTE value );
	BOOL SetColorBlue( BYTE value );

	BOOL ChangeMode( BOOL bSet );
	BOOL GetMessage( CString& message );

	void HioCallback();

private:
	BOOL WriteToMemory( DWORD addr, BYTE offset, BYTE data );
	BOOL ReadFromMemory( DWORD addr, BYTE* data );
};

#endif // !defined(AFX_HIOACCESS_H__A33EE737_EB7A_11D4_85C5_0090CC011405__INCLUDED_)
