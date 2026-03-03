#include "controls.h"
#include "Utils\mutils.h"
#include "MGTools\Include\Utils\Utils.h"
#include "ImageTempl\ImageDoc.h"
#include "ImageTempl\ImageView.h"

CControls::CControls()
{
   Init();
}

CControls::~CControls()
{
  ImagePathMap.RemoveAll();
  SaveINISettings();
}

void CControls::Init()
{
  MarkerSide = defMarkerSide;
  DotSide = defDotSide;
  ImagePathMap.RemoveAll();
  FringeCenterAs = FC_MAX;
  ActiveEditMode = -1;
  EnableTracker = FALSE;
  EnableCustomDots = FALSE; 
  CurTypeBound = BOUND_ELLIPSE;
  ViewState = V_INTERFEROGRAM|V_EXTREMUMS|V_ZAPSECTIONS|V_DOTLINES|V_DOTS;
  IsAdvBar = 1;
  MaxPow = 8;
  Eps = 1.e-5;
  COLORREF Color;
//   if(ReadSection("FRAGMENTCOLOR", "Color1", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(255, 128, 192);//малин
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color2", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(102, 153, 255);//голуб-серый
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color3", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(255, 0, 204);//малин
     ObjColors.Add(Color);
    
//   if(ReadSection("FRAGMENTCOLOR", "Color4", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(255, 153, 0);//оранж
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color5", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(102, 204, 102);//зел
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color6", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(153, 51, 153);//фиолет
    ObjColors.Add(Color);
    
//   if(ReadSection("FRAGMENTCOLOR", "Color7", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(255, 153, 153);//свет кирпич
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color8", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(204, 102, 102);//коричневый
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color9", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(255, 153, 153);//свет кирпич
    ObjColors.Add(Color);

//   if(ReadSection("FRAGMENTCOLOR", "Color10", tCol, GetIniFile()))
//        Color = atoi(tCol);
//   else
      Color = RGB(51, 255, 255);//голубой
    ObjColors.Add(Color);
}

void CControls::LoadINISettings()
{
  TCHAR text[50];
  if(ReadSection("VIEW", "State", text, GetIniFile()))
	ViewState = atoi(text);  
  else	  
    ViewState = V_INTERFEROGRAM|V_EXTREMUMS|V_ZAPSECTIONS|V_DOTLINES|V_DOTS;
  if(ReadSection("VIEW", "AdvBar", text, GetIniFile()))
	IsAdvBar = atoi(text);
  else
	IsAdvBar = 1; 
}

void CControls::SaveINISettings()
{
  CString s;
  s.Format("%d", ViewState);
  SaveSection("VIEW", "State", LPCTSTR(s), GetIniFile());
  s.Format("%d", IsAdvBar);
  SaveSection("VIEW", "AdvBar", LPCTSTR(s), GetIniFile());
}

void CControls::LoadBoundSettings()
{
  TCHAR text[50];
  if(ReadSection("BOUNDS", "Tracker", text, GetIniFile()))
   EnableTracker = (BOOL)(atoi(text));
  else	  
   EnableTracker = FALSE;
  
  if(ReadSection("BOUNDS", "CustomDots", text, GetIniFile()))
   EnableCustomDots = (BOOL)(atoi(text));
  else	  
   EnableCustomDots = FALSE; 

  if(ReadSection("BOUNDS", "CurType", text, GetIniFile()))
   CurTypeBound = atoi(text);
  else	  
   CurTypeBound = BOUND_ELLIPSE;
}

void CControls::SaveBoundSettings()
{
  CString s;
  s.Format("%d", (int)EnableTracker);
  SaveSection("BOUNDS", "Tracker", LPCTSTR(s), GetIniFile());
  s.Format("%d", (int)EnableCustomDots);
  SaveSection("BOUNDS", "CustomDots", LPCTSTR(s), GetIniFile());
  s.Format("%d", CurTypeBound);
  SaveSection("BOUNDS", "CurType", LPCTSTR(s), GetIniFile());
}

bool CControls::GetIndexColor(double nFringe, COLORREF& Color)
{
	int idx = (int)(fabs(nFringe)+0.5);
	int maxIdx = ObjColors.GetSize()-1;
	int n_maxIdx = int(idx/maxIdx);
	int i = idx - maxIdx*n_maxIdx;
	Color = ObjColors[i];
	return true;
}

bool CControls::GetCorrectDotSize(int& dotSide, CImageDoc* pActDoc)
{
    CSize side = DotSide;
	pActDoc->GetView()->ClientToDoc(side);
	dotSide = side.cx; 
	if(dotSide>DotSide) dotSide = DotSide;
	if(dotSide<4) dotSide = 4;
	return true;
}

bool CControls::GetUniqueImagePath(CString& uniquePath, LPCTSTR realPath)
{
	CString s = realPath;
	CString path, name;
	int iP1 = s.Find('(');
	int iP2 = s.Find(')');
	if(iP1 != -1 && iP2 != -1){
      s = s.Left(s.GetLength()-7);
	}
	else{
      s = s.Left(s.GetLength()-4);
	}
	int iP = s.ReverseFind('\\');
	path = s.Left(iP+1);
	name = s.Mid(iP+1);
	int nCopy;
	BOOL res = ImagePathMap.Lookup(LPCTSTR(name), (CObject*&)nCopy);
    if(!res){
		nCopy = 1;
        ImagePathMap.SetAt(LPCTSTR(name), (CObject*&)nCopy);
		name += "(1).bmp";
		uniquePath = path+name;
	    return true;
    }
	else{
		nCopy++;
        ImagePathMap.SetAt(LPCTSTR(name), (CObject*&)nCopy);
		s.Format("(%d).bmp", nCopy);
		name += s;
		uniquePath = path+name;
	    return true;
	}

}

bool CControls::GetCorrectFilterForSave(CString& str, int& nFilters)
{
    CDigitInfo* pDI = GetDigitInfo();
    LPCTSTR lan;
	str.Empty();
	nFilters = 0;
	
	if(pDI->IsDots()){
     lan = "Digit Interferogram Files (FRN)";
     str += lan; str += (TCHAR)NULL;
     str += "*.frn"; str += (TCHAR)NULL;
	 nFilters += 1;
	 lan = "FinFringe Interferogram Files (FRN)";
	 str += lan; str += (TCHAR)NULL;
	 str += "*.frn"; str += (TCHAR)NULL;
	 nFilters += 1;
	 lan = "Phase Map (MTR)";
	 str += lan; str += (TCHAR)NULL;
	 str += "*.mtr"; str += (TCHAR)NULL;
	 nFilters += 1;
	}
    if(pDI->IsZapSections()){
     lan = "Files (ZAP)";
     str += lan; str += (TCHAR)NULL;
     str += "*.zap"; str += (TCHAR)NULL;
	 nFilters += 1;

     lan = "Files old format(ZAP)";
     str += lan; str += (TCHAR)NULL;
     str += "*.zap"; str += (TCHAR)NULL;
	 nFilters += 1;
	}
	if(nFilters)
		return true;
	else
		return false;
}

void CControls::OnOpenImageDocument()
{
	CDocument* pDoc = GetWIActiveDocument();
	if(!pDoc){
      GetMainFrame()->ShowMeasurePane(FALSE);
	  EnableOptions = NULL;
	}
}

void CControls::OnCloseImageDocument()
{
	CDocument* pDoc = GetWIActiveDocument();
	if(!pDoc){
      GetMainFrame()->ShowMeasurePane(FALSE);
	  EnableOptions = NULL;
	}
}

DigitMode::FringeEditMode CControls::GetEditMode()
{
	/*
#define E_ADD_DOT        0
#define E_DELETE_DOT     1
#define E_DELETE_FRINGE  2
#define E_ADD_SECTION    3
#define E_DELETE_SECTION 4
#define E_SECTION_LEFT   5
#define E_SECTION_RIGHT  6
#define E_RENUM_DOT      7
#define E_RENUM_FRINGE   8

	*/
	switch (ActiveEditMode) {
	  case E_ADD_SECTION:
		return DigitMode::FringeEditMode::Navigate;
	  case E_ADD_DOT:
		return DigitMode::FringeEditMode::Draw;
	  case E_RENUM_DOT:
		return DigitMode::FringeEditMode::DotEdit;
	  default:
		return DigitMode::FringeEditMode::Navigate;
	}
}