//By: George Polouse

// Tables.cpp: implementation of the CTables class.
//

#include "stdafx.h"
#include "Tables.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

IMPLEMENT_DYNAMIC(CTables, CRecordsetEx)

CTables::CTables(CDatabase* pDatabase)
	: CRecordsetEx(pDatabase)
{
	m_strTableQualifier = _T("");
	m_strTableOwner     = _T("");
	m_strTableName      = _T("");
	m_strTableType      = _T("");
	m_strRemarks        = _T("");
	m_nFields = 5;
}

BOOL CTables::Open(LPCSTR pszTableQualifier,
	LPCSTR pszTableOwner, LPCSTR pszTableName ,LPCSTR pszTableType,
	UINT nOpenType)
{
	RETCODE nRetCode;
	UWORD   bFunctionExists;

	// Make sure SQLTables exists
	AFX_SQL_SYNC(::SQLGetFunctions(m_pDatabase->m_hdbc,
		SQL_API_SQLTABLES,&bFunctionExists));
	if(!Check(nRetCode))
		AfxThrowDBException(nRetCode, m_pDatabase, m_hstmt);
	if(!bFunctionExists)
		throw _T("<::SQLTables> not supported.");

	// Cache state info and allocate hstmt
	SetState(nOpenType,NULL,readOnly);
	if (!AllocHstmt())
		return FALSE;

	TRY
	{
		OnSetOptions(m_hstmt);
		AllocStatusArrays();

		// Call the ODBC function
		AFX_ODBC_CALL(::SQLTables(m_hstmt,
			(UCHAR FAR*)pszTableQualifier,SQL_NTS,
			(UCHAR FAR*)pszTableOwner,SQL_NTS,
			(UCHAR FAR*)pszTableName,SQL_NTS,
			(UCHAR FAR*)pszTableType,SQL_NTS));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode,m_hstmt);

		// Allocate memory and cache info
		AllocAndCacheFieldInfo();
		AllocRowset();

		// Fetch the first row of data
		MoveNext();

		// If EOF, result set is empty, set BOF as well
		m_bBOF = m_bEOF;

	}

	CATCH_ALL(e)
	{
		Close();
		THROW_LAST();
	}
	END_CATCH_ALL

	return TRUE;
}

void CTables::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX,_T("TABLE_QUALIFIER"),m_strTableQualifier);
	RFX_Text(pFX,_T("TABLE_OWNER"),m_strTableOwner);
	RFX_Text(pFX,_T("TABLE_NAME"),m_strTableName);
	RFX_Text(pFX,_T("TABLE_TYPE"),m_strTableType);
	RFX_Text(pFX,_T("REMARKS"),m_strRemarks);
}

CString CTables::GetDefaultConnect()
{ 
	return _T("ODBC;");
}

CString CTables::GetDefaultSQL()
{ 
	return "!"; // Direct ODBC call
}

/////////////////////////////////////////////////////////////////////////////
// CTables diagnostics

#ifdef _DEBUG
void CTables::AssertValid() const
{
	CRecordsetEx::AssertValid();
}

void CTables::Dump(CDumpContext& dc) const
{
	CRecordsetEx::Dump(dc);
}
#endif //_DEBUG