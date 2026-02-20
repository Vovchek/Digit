#if !defined(AFX_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_

#include "stdafx.h"
#include "Appdef.h"
#include "DigitMode/InputHandler.h"

class CImageDoc;
class CControls
{
 public:
   CMapStringToOb ImagePathMap;
   DWORD EnableOptions;
   BOOL EnableTracker; 
   BOOL EnableCustomDots; 
   int CurTypeBound;
   CStringList curScenario;
   CArray<COLORREF, COLORREF> ObjColors;
   int MarkerSide;
   int DotSide;
   int ActiveEditMode;
   int FringeCenterAs;
   UINT ViewState;
   UINT IsAdvBar;

   int MaxPow;
   double Eps;
   
 public:
    CControls();
    ~CControls();
    void Init();
    void OnOpenImageDocument();
	void OnCloseImageDocument();
	bool GetUniqueImagePath(CString& uniquePath, LPCTSTR realPath);
	bool GetIndexColor(double nFringe, COLORREF& Color);
    bool GetCorrectDotSize(int& dotSide, CImageDoc* pActDoc);
	void LoadINISettings();
	void SaveINISettings();
	void LoadBoundSettings();
	void SaveBoundSettings();
    bool GetCorrectFilterForSave(CString& str, int& nFilters);
	DigitMode::FringeEditMode GetEditMode();
};

#endif // !defined(AFX_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)

