#if !defined(AFX_M_UTILS_H__7A162971_1875_11D4_8A51_A0D170A2AAD8__INCLUDED_)
#define AFX_M_UTILS_H__7A162971_1875_11D4_8A51_A0D170A2AAD8__INCLUDED_

#include <io.h>
#include "MGTools\StdAfx.h"
#include "Digit.h"
#include "MainFrame\MainFrm.h"
#include "Controls\ImageCtrls.h"
#include "Controls\MeasureCtrls.h"
#include "Controls\BoundCtrls.h"
#include "DigitMode\DigitInfo.h"
#include "MGTools\Include\Utils\BaseDataType.h"
//C:\Ilya\Programming\cpp\Numbering\Utils\mutils.h

namespace DigitMode {
    class CApertureCtrls;
}

LPCTSTR GetIniFile();
LPCTSTR GetWorkPath();
LPCTSTR GetWorkFolder();
void GetImageFileName(CString& PathName, CSize& cs);

CMainFrame* GetMainFrame();
CToolBar* GetImageToolBar();
CDocument* GetWIActiveDocument();

void MinimizeUnactiveDocuments(bool minimize);
void DeActivateMode(UINT iMode);
bool IsLastActiveDoc();
void UpdateAllImageViews();
BOOL CheckForSaveAllImageDoc();

CControls* GetControls();
CImageCtrls* GetImageCtrls(CView* pView=NULL);
CMeasureCtrls* GetMeasureCtrls(CView* pView=NULL);
CBoundCtrls* GetBoundCtrls(CView* pView=NULL);
DigitMode::CApertureCtrls* GetApertureCtrls(CView* pView=NULL);
CDigitInfo* GetDigitInfo(CView* pView=NULL);

bool MakeCopyDocument(LPCTSTR InitPath, LPCTSTR InitName, LPCTSTR ActivePath);

CSingleDocTemplate* GetTemplTextInfo();
CMultiDocTemplate* Get3DTempl();
CDocument* GetDocTextInfo();

LPCTSTR CRS(LPCTSTR rusLang, LPCTSTR engLang);
LPTSTR CRS(LPTSTR rStr, LPTSTR eStr, int /*a*/);

BOOL IsRussianLang();

BOOL InitSection(LPCTSTR _secName, LPCTSTR _iniFile);
BOOL LoadKeySection(LPCTSTR _secName,LPCTSTR _Key, CString& Text, LPCTSTR _iniFile,  BOOL cutComments=TRUE);
BOOL SaveKeySection(LPCTSTR _secName,LPCTSTR _Key, CString Text, LPCTSTR _iniFile, BOOL saveComments=TRUE);

BOOL BeginNotePadProcess(LPCTSTR strCommand);
void TerminateAllNotePadProcess();

	
#endif // !defined(AFX_M_UTILS_H__7A162971_1875_11D4_8A51_A0D170A2AAD8__INCLUDED_)
