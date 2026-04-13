// CCommentsFile.cpp : implementation file
//

#include "stdafx.h"
#include "digit.h"
#include "CommentsFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommentsFile dialog


CCommentsFile::CCommentsFile(CWnd* pParent /*=NULL*/)
	: CPaneDialog()
{
	//{{AFX_DATA_INIT(CCommentsFile)
		// NOTE: the ClassWizard will add member initialization here
	Comments = _T(" ");
	ScaleFactor = 1.;
	Rotation = 0.;
	//}}AFX_DATA_INIT
}


void CCommentsFile::DoDataExchange(CDataExchange* pDX)
{
	CPaneDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommentsFile)
		// NOTE: the ClassWizard will add DDX and DDV calls here
    DDX_Text(pDX, IDE_comments_info, Comments);
    DDX_Text(pDX, IDE_scalefactor, ScaleFactor);
    DDX_Text(pDX, IDE_rotation, Rotation);
	//}}AFX_DATA_MAP
}

void CCommentsFile::SetComments(LPCTSTR _Comments)
{
	Comments = _Comments;
}

void CCommentsFile::SetScaleFactor(double _ScaleFactor)
{
	ScaleFactor = _ScaleFactor;
}

void CCommentsFile::SetRotation(double _Rotation)
{
	Rotation = _Rotation;
}

void CCommentsFile::GetComments(CString& _Comments)
{
	_Comments = Comments;
}

void CCommentsFile::GetScaleFactor(double& _ScaleFactor)
{
	_ScaleFactor = ScaleFactor;
}

void CCommentsFile::GetRotation(double& _Rotation)
{
	_Rotation = Rotation;
}

BEGIN_MESSAGE_MAP(CCommentsFile, CPaneDialog)
	//{{AFX_MSG_MAP(CCommentsFile)
    ON_EN_CHANGE(IDE_comments_info, OnChangeComments)
    ON_EN_CHANGE(IDE_scalefactor, OnChangeScaleFactor)
    ON_EN_CHANGE(IDE_rotation, OnChangeRotation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommentsFile message handlers

void CCommentsFile::OnChangeComments()
{
}

void CCommentsFile::OnChangeScaleFactor()
{
}

void CCommentsFile::OnChangeRotation()
{
}
