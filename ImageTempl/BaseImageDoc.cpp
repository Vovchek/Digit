// BaseImageDoc.cpp : implementation file
//
/***********************************************************************************
 МОДУЛЬ: BaseImageDoc.cpp
 НАЗНАЧЕНИЕ:
         Базовый документ архитектуры документ-представления      
************************************************************************************/

#include "stdafx.h"
#include "BaseImageDoc.h"
#include "BaseImageView.h"
#include "ImageChildFrm.h"

#include "Controls\ImageCtrls.h"
#include "Utils\mutils.h"
#include "MGTools\Include\Utils\Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseImageDoc

IMPLEMENT_DYNCREATE(CBaseImageDoc, CDocument)

CBaseImageDoc::CBaseImageDoc() : imageCtrls()
, apertureCtrls(imageCtrls) // interface dependency
{
}

//Инициализация данных документа  
void CBaseImageDoc::InitData()
{
  //boundCtrls.Init();
}

// Смотри Microsoft Visual C++ документацию
BOOL CBaseImageDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CBaseImageDoc::~CBaseImageDoc()
{
}

// Смотри Microsoft Visual C++ документацию
CBaseImageView* CBaseImageDoc::GetView()
{
   POSITION pos = GetFirstViewPosition();
   while (pos != NULL)
   {
      CView* pView = GetNextView(pos);
      if(pView->IsKindOf(RUNTIME_CLASS(CBaseImageView)))
          return (CBaseImageView*) pView;
   }
   return NULL;
}


BEGIN_MESSAGE_MAP(CBaseImageDoc, CDocument)
	//{{AFX_MSG_MAP(CBaseImageDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
    ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseImageDoc diagnostics

// Смотри Microsoft Visual C++ документацию
#ifdef _DEBUG
void CBaseImageDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBaseImageDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBaseImageDoc serialization

// Смотри Microsoft Visual C++ документацию
void CBaseImageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBaseImageDoc commands
// Смотри Microsoft Visual C++ документацию
BOOL CBaseImageDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	TRACE("CBaseImageDoc::OnOpenDocument(%s)\n", lpszPathName ? lpszPathName : "NULL");

	if (!CDocument::OnOpenDocument(lpszPathName))
	{
		TRACE("CBaseImageDoc::OnOpenDocument - returning FALSE (base class failed)\n");
		return FALSE;
	}

	InitData();
	TRACE("CBaseImageDoc::OnOpenDocument - returning TRUE\n");
	return TRUE;
}

// Инициализация и загрузка изображения
BOOL CBaseImageDoc::ReloadDocument(LPCTSTR lpszImagePathName)
{
	TRACE("CBaseImageDoc::ReloadDocument(%s)\n", lpszImagePathName ? lpszImagePathName : "NULL");

	CString fname = lpszImagePathName;
	if(fname.IsEmpty())
	{
		TRACE("CBaseImageDoc::ReloadDocument - returning FALSE (empty filename)\n");
		return FALSE;
	}

	int iP = fname.ReverseFind('\\');
	RealPath = fname.Left(iP+1);
	RealName = fname.Mid(iP+1);

	if(fname.IsEmpty())
	{
		TRACE("CBaseImageDoc::ReloadDocument - returning TRUE (fname empty after parsing)\n");
		return TRUE;
	}

	imageCtrls.OriginalPath = fname;
	if(!imageCtrls.LoadImage(fname)){
		CString mes;
		mes = CRS("Невозможно открыть файл\r\n", "Fail to load file\r\n");
		mes += fname;
		imageCtrls.OriginalPath.Empty();
		AfxMessageBox(LPCTSTR(mes));
		TRACE("CBaseImageDoc::ReloadDocument - returning FALSE (LoadImage failed)\n");
		return FALSE;
	}

	if(imageCtrls.m_pDIB->m_nSrcBitsPerPixel != 8){
		CWindowDC* pDC = new CWindowDC(GetView());
		imageCtrls.ConvertToGrayScale(pDC, LPCTSTR(fname));
		delete pDC;
	}
 
	CString s = fname;
	if(s.Find("_u.bmp") !=-1){
		s = s.Left(s.GetLength()-6);
	}
	else{
		s = s.Left(s.GetLength()-4);
	}
	TmpPath = s + ".bmp";
	UndoTmpPath = s + "_u.bmp";
  
	TRACE("CBaseImageDoc::ReloadDocument - returning TRUE\n");
	return TRUE;
}
// Копирование файла изображения в TEMP директорию
void CBaseImageDoc::CopyImage()
{
   CImageChildFrm* pFr = (CImageChildFrm*)(GetView()->GetParentFrame());
   pFr->SavePlacement();
   MakeCopyDocument(LPCTSTR(RealPath), LPCTSTR(RealName), LPCTSTR(TmpPath));
}
// Возвращает путь к TEMP директории
LPCTSTR CBaseImageDoc::GetTmpPath()
{
	return LPCTSTR(TmpPath);
}

// Возвращает путь к UNDO файлу
LPCTSTR CBaseImageDoc::GetUndoTmpPath()
{
	return LPCTSTR(UndoTmpPath);
}

// Возвращает имя оригинала файла изображения
LPCTSTR CBaseImageDoc::GetRealName()
{
	return LPCTSTR(RealName);
}

// Возвращает путь к оригиналу файла изображения
LPCTSTR CBaseImageDoc::GetRealPath()
{
	return LPCTSTR(RealPath);
}

// Изменяет путь к оригиналу файла изображения
void CBaseImageDoc::SetRealPath(LPCTSTR _RealPath)
{
	RealPath = _RealPath;
}

// Изменяет имя оригинала файла изображения
void CBaseImageDoc::SetRealName(LPCTSTR _RealName)
{
	RealName = _RealName;
}

// Удаляет параметры документа
void CBaseImageDoc::DeleteContents() 
{
	if(!TmpPath.IsEmpty())
	  ::DeleteFile(LPCTSTR(TmpPath));
	if(!UndoTmpPath.IsEmpty())
	  ::DeleteFile(LPCTSTR(UndoTmpPath));
	
	CDocument::DeleteContents();
}

// Смотри Microsoft Visual C++ документацию
BOOL CBaseImageDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
    imageCtrls.SaveImage(lpszPathName);
	return TRUE;
}

void CBaseImageDoc::OnFileClose()
{
}

void CBaseImageDoc::OnFileSave()
{
}

// Смотри Microsoft Visual C++ документацию
void CBaseImageDoc::OnFileSaveAs()
{
    LPCTSTR title = CRS("Сохранить файл", "Save file");
    CFileDialog fileDlg(FALSE);

    LPCTSTR lan;
	CString str;
    str.Empty();
    lan = CRS("Файлы (BMP)","Files (BMP)");
    str += lan; str += (TCHAR)NULL;
    str += "*.bmp"; str += (TCHAR)NULL;
    lan = CRS("Файлы (PCX)","Files (PCX)");
    str += lan; str += (TCHAR)NULL;
    str += "*.pcx"; str += (TCHAR)NULL;
    lan = CRS("Файлы (JPG)","Files (JPG)");
    str += lan; str += (TCHAR)NULL;
    str += "*.jpg"; str += (TCHAR)NULL;
    lan = CRS("Файлы (TIF)","Files (TIF)");
    str += lan; str += (TCHAR)NULL;
    str += "*.tif"; str += (TCHAR)NULL;
    fileDlg.m_ofn.lpstrFilter = LPCTSTR(str);
    fileDlg.m_ofn.nFilterIndex = 1;

    TCHAR strName[_MAX_PATH];
    strName[0] = (TCHAR)NULL;
	CString s;
	int iP = TmpPath.ReverseFind('\\');
	s = TmpPath.Mid(iP+1);
	s = s.Left(s.GetLength()-4);
    strcpy(strName, LPCTSTR(s));
    fileDlg.m_ofn.lpstrFile = strName;
    fileDlg.m_ofn.lpstrTitle = title;

    fileDlg.m_ofn.Flags |= (OFN_EXTENSIONDIFFERENT|OFN_OVERWRITEPROMPT);
    TCHAR Str[_MAX_PATH];
    if(ReadPath(GetWorkPath(), Str, GetIniFile()))
      fileDlg.m_ofn.lpstrInitialDir = Str;
    else
      fileDlg.m_ofn.lpstrInitialDir = NULL;
    if(fileDlg.DoModal() == IDOK){
     CString FileName = fileDlg.GetPathName();
     int iP = FileName.ReverseFind('\\');
     CString sav = FileName.Left(iP);
     SavePath(GetWorkPath(), sav, GetIniFile());
	 CString Ext = fileDlg.GetFileExt();
	 if(Ext.IsEmpty()){
		if(fileDlg.m_ofn.nFilterIndex == 1) 
		 FileName += ".bmp";
		else if(fileDlg.m_ofn.nFilterIndex == 2) 
		 FileName += ".pcx";
		else if(fileDlg.m_ofn.nFilterIndex == 3) 
		 FileName += ".jpg";
		else if(fileDlg.m_ofn.nFilterIndex == 4) 
		 FileName += ".tif";
	 }
	 
     CDocument* pDoc = GetWIActiveDocument();
	 pDoc->OnSaveDocument(LPCTSTR(FileName));
	}
}

// Добавление строки в сценарий обработки
// idCom - идентификатор комманды
// _Options - список параметров
void CBaseImageDoc::AddRecordScenario(int idCom, LPCTSTR _Options)
{
    CString Command;
	CString Options;
	CString Record;
	Options = " (";
	Options += _Options;
	Options += ")";
	
	switch(idCom){
	    case O_BRITGHTNESS_CONTRAST:
			Command = "BRITGHTNESS_CONTRAST";
		  break;
	    case O_ROTATION:
			Command = "ROTATION";
		  break;
	    case O_GAUSS:
			Command = "GAUSS";
		  break;
	    case O_INCLINE_CORRECT:
			Command = "INCLINE_CORRECT";
		  break;
	    case O_ZOOM:
			Command = "ZOOM";
		  break;
	    case O_GAMMA:
			Command = "GAMMA";
		  break;
	    case O_RATIO_SIDE:
			Command = "RATIO_SIDE";
		  break;
	    case O_CROP:
			Command = "CROP";
		  break;
	    case O_EXPAND:
			Command = "EXPAND";
		  break;
	    case O_ROTATION_90:
			Command = "ROTATION_90";
		  break;
	    case O_ROTATION_A90:
			Command = "ROTATION_A90";
		  break;
	    case O_FLIP_H:
			Command = "FLIP_H";
		  break;
	    case O_FLIP_V:
			Command = "FLIP_V";
		  break;
	    case O_INVERT:
			Command = "INVERT";
		  break;
	}
	
  Record = Command + Options;	
  Scenario.AddTail(Record);
}

// Удаляет последнюю запись сценария обработки
void CBaseImageDoc::RemoveLastRecordScenario()
{
	Scenario.RemoveTail();
}

//Присваивает сценария обработки данному документу
//newScenario - новый сценарий
void CBaseImageDoc::SetScenario(CStringList& newScenario)
{
  Scenario.RemoveAll();
  CString s;	
  POSITION pos = newScenario.GetHeadPosition();
  for(int i=0;i < newScenario.GetCount();i++){
     s = newScenario.GetNext(pos);
	 Scenario.AddTail(s);
  }
}

//Возвращает сценарий обработки данного документа
//retScenario - сценарий данного документа
void CBaseImageDoc::GetScenario(CStringList& retScenario)
{
  retScenario.RemoveAll();
  CString s;	
  POSITION pos = Scenario.GetHeadPosition();
  for(int i=0;i < Scenario.GetCount();i++){
     s = Scenario.GetNext(pos);
	 retScenario.AddTail(s);
  }
}

