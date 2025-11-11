#include "..\..\Include\Utils\utils.h"
#include <direct.h>

BOOL ReadWindowPlacement(LPWINDOWPLACEMENT pwp)
{
	char szSection[] = "Settings";
	char szWindowPos[] = "WindowPos";
	char szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");

	CString strBuffer = AfxGetApp()->GetProfileString(szSection, szWindowPos);
	if (strBuffer.IsEmpty())
		return FALSE;

	WINDOWPLACEMENT wp;
	int nRead = _stscanf(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	if (nRead != 10)
		return FALSE;

	wp.length = sizeof wp;
	*pwp = wp;
	return TRUE;
}

void WriteWindowPlacement(LPWINDOWPLACEMENT pwp)
	// write a window placement to settings section of app's ini file
{
	char szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];
	char szSection[] = _T("Settings");
	char szWindowPos[] = _T("WindowPos");
	char szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");

	wsprintf(szBuffer, szFormat,
		pwp->flags, pwp->showCmd,
		pwp->ptMinPosition.x, pwp->ptMinPosition.y,
		pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
		pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
		pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
	AfxGetApp()->WriteProfileString(szSection, szWindowPos, szBuffer);
}

TCHAR szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");
TCHAR strBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

BOOL ReadWndPlacement(LPWINDOWPLACEMENT pwp,
                      LPCTSTR Section, LPCTSTR Key, LPCTSTR File)
{
    LPCTSTR Def = "ERROR";
    ::GetPrivateProfileString(Section, Key, Def, strBuffer,
                             sizeof("-32767")*8 + sizeof("65535")*2, File);
    if (CString(strBuffer).IsEmpty())
         return FALSE;

    WINDOWPLACEMENT wp;
    int nRead = _stscanf(strBuffer, szFormat,
        &wp.flags, &wp.showCmd,
        &wp.ptMinPosition.x, &wp.ptMinPosition.y,
        &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
        &wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
        &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

    if (nRead != 10)
        return FALSE;

    wp.length = sizeof wp;
    *pwp = wp;
    return TRUE;
}

BOOL WriteWndPlacement(LPWINDOWPLACEMENT pwp,
                       LPCTSTR Section, LPCTSTR Key,
                       LPCTSTR File)
{
   wsprintf(strBuffer, szFormat,
        pwp->flags, pwp->showCmd,
        pwp->ptMinPosition.x, pwp->ptMinPosition.y,
        pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
        pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
        pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);

   CString Str = Key;
   int iPos = Str.Find("-");
   if(iPos != -1) Str = Str.Mid(0,iPos-1);
   return ::WritePrivateProfileString(Section, LPCTSTR(Str), strBuffer, File);
}

//=========================================================================
double ReadStatic(CWnd *pParent, int Id)
{
	CWnd *pWnd=pParent->GetDlgItem(Id);
	CString buf;
	pWnd->GetWindowText(buf);
	return atof(buf);
}
//=========================================================================
void EnableControl(CWnd *pDialog, int IdBtn, BOOL bState)
{
	CWnd *pWnd=pDialog->GetDlgItem(IdBtn);
	pWnd->EnableWindow(bState);
}
//-------------------------------------------------------------------------
BOOL DeleteFile(CString FileName)
{
	return remove(FileName)==0;
}
static BOOL GetRealWindowsVersion(DWORD& major, DWORD& minor, DWORD& build)
{
	typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
	HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
	if (!ntdll)
		return FALSE;

	RtlGetVersionPtr pRtlGetVersion =
		(RtlGetVersionPtr)::GetProcAddress(ntdll, "RtlGetVersion");
	if (!pRtlGetVersion)
		return FALSE;

	RTL_OSVERSIONINFOW ver = { sizeof(ver) };
	if (pRtlGetVersion(&ver) != 0)
		return FALSE;

	major = ver.dwMajorVersion;
	minor = ver.dwMinorVersion;
	build = ver.dwBuildNumber;
	return TRUE;
}
double GetWindowsVersion()
{
	char szBuf[10];
#if _WIN32_WINNT < 0x0601
	DWORD dwVersion = GetVersion();

	sprintf(szBuf, "%d.%d", LOBYTE(LOWORD(dwVersion)),
									HIBYTE(LOWORD(dwVersion)));
#else
	DWORD major = 0, minor = 0, build = 0;
	GetRealWindowsVersion(major, minor, build);
	sprintf(szBuf, "%d.%d", major, minor);
#endif
	return atof(szBuf);
}

void GetStandardValue(double Value, double &MinValue, double &MaxValue)
{
	double ValueStandard[]={1., 1.5, 2., 2.5, 4., 5., 8.};
	int NValuesStandard=sizeof(ValueStandard)/sizeof(double);

	double SignValue=(Value<0)? -1. : 1.;
	Value=fabs(Value);

	double Log10_Value=log10(Value);
	Log10_Value=Log10_Value<0? int(Log10_Value)-1 : int(Log10_Value);

	double NewValue=Value/pow(10., Log10_Value);
	double DeltaMin=10;
	int iMin=0;
	for(int i=0; i<7; i++) {
		if(fabs(NewValue-ValueStandard[i])<DeltaMin) {
			DeltaMin=fabs(NewValue-ValueStandard[i]);
			iMin=i;
		}
	}
	MinValue=ValueStandard[iMin]*pow(10, Log10_Value);

	if(iMin>=NValuesStandard-1) MaxValue=ValueStandard[0]*10*pow(10, Log10_Value);
	else MaxValue=ValueStandard[iMin+1]*pow(10, Log10_Value);

	if(SignValue<0) {
		double Work=MinValue;
		MinValue=-MaxValue;
		MaxValue=-Work;
	}
}

void GetMax(int n, double* pArr,
			int& iMax, double& yMax)
{
	yMax= -FLT_MAX;

	for(int i= 0; i<n; i++) {
		if(yMax < pArr[i]) {
			iMax= i;
			yMax= pArr[i];
		}
	}
}
//-------------------------------------------------------------------------------------------------
void GetMax(int n, double* pArr, int i1, int i2,
			int& iMax, double& yMax)
{
	yMax= -FLT_MAX;

	for(int i= i1; i<i2; i++) {
		if(yMax < pArr[i]) {
			iMax= i;
			yMax= pArr[i];
		}
	}
}
//-------------------------------------------------------------------------------------------------
void GetMin(int n, double* pArr, int& iMin, double& yMin)
{
	yMin= FLT_MAX;

	for(int i= 0; i<n; i++) {
		if(yMin > pArr[i]) {
			iMin= i;
			yMin= pArr[i];
		}
	}
}
//-------------------------------------------------------------------------------------------------
void GetMin(int n, double* pArr, int i1, int i2,
			int& iMin, double& yMin)
{
	yMin= FLT_MAX;

	for(int i= i1; i<i2; i++) {
		if(yMin > pArr[i]) {
			iMin= i;
			yMin= pArr[i];
		}
	}
}
//-------------------------------------------------------------------------------------------------
void _MinMax(double *M, int N, double &Min, double &Max)
{
	Min=Max=M[0];
	for(int x=1; x<N; x++) {
		if(M[x]<Min) Min=M[x];
		if(M[x]>Max) Max=M[x];
	}
}
//-------------------------------------------------------------------------------------------------
CString GetAppName(void)
{
	char Filename[256];
	::GetModuleFileName(NULL, Filename, 256);
	CString FullFileName= Filename;
	return FullFileName;
}
//-------------------------------------------------------------------------------------------------
CString GetFilePath(CString FullFileName)
{
	int Ind= FullFileName.ReverseFind('\\');
	CString FilePath= FullFileName.Left(Ind);
	return FilePath;
}
//-------------------------------------------------------------------------------------------------
CString GetFileNameWithoutExt(CString FullFileName)
{
    CString FileName= GetFileName(FullFileName);
	int Ind= FileName.ReverseFind('.');
	CString NameWithoutExt;
	if(Ind != -1) NameWithoutExt= FileName.Left(Ind);
	else NameWithoutExt= FileName;

	return NameWithoutExt;
}
//-------------------------------------------------------------------------------------------------
CString GetFileExtension(CString FullFileName)
{
	CString FileName= GetFileName(FullFileName);
	int Ind= FileName.ReverseFind('.');
	if(Ind == -1) {
		CString Ext= "";
		return Ext;
	}

	Ind= FileName.GetLength() - Ind - 1;
	CString Ext= FileName.Right(Ind);
	return Ext;
}
//-------------------------------------------------------------------------------------------------
CString GetFileName(CString FullFileName)
{
	int Ind= FullFileName.ReverseFind('\\');
	Ind= FullFileName.GetLength() - Ind - 1;
	CString FileName= FullFileName.Right(Ind);
	return FileName;
}
//-------------------------------------------------------------------------------------------------
CString GetAppPath(void)
{
	return GetFilePath(GetAppName());
}
//-------------------------------------------------------------------------------------------------
int GetFilesInDir(CString& Dir, CString Filter, MemString& Files)
{
	struct _finddata_t  ffblk;

	CString PathName= Dir + "\\" + Filter;
	long handle= _findfirst(LPCSTR(PathName), &ffblk);//, FA_RDONLY|FA_ARCH);

	if (handle == -1)
		return 0;

	int Number=0;
    int res= 0;
	while(res != -1)
	{
		Number++;
		res= _findnext(handle, &ffblk);
	}

	if(Number==0) return 0;

	Files.Set(Number);

	handle= _findfirst(LPCSTR(PathName), &ffblk);//, FA_RDONLY|FA_ARCH);
	Files(0)= Dir + "\\" + ffblk.name;
	for(int i=1; i<Number; i++) {
		_findnext(handle, &ffblk);
		Files(i)= Dir + "\\" + ffblk.name;
	}

	return Number;
}
//-------------------------------------------------------------------------------------------------
void SetCursorArrow(void)
{
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}
//-------------------------------------------------------------------------------------------------
void SetCursorWait(void)
{
	::SetCursor(::LoadCursor(NULL, IDC_WAIT));
}
//-------------------------------------------------------------------------------------------------
int GetMDIChildNumber(CMDIFrameWnd* pWnd)
{
	int nChild= 0;
	HWND hWndChild= ::GetTopWindow(pWnd->m_hWndMDIClient);
	while(hWndChild) {
		nChild++;
		hWndChild= ::GetNextWindow(hWndChild, GW_HWNDNEXT);
	}

	return nChild;
}
//-------------------------------------------------------------------------------------------------
void RestoreObjects(CDC& dc)
{
	dc.SelectStockObject(DEFAULT_PALETTE);
	dc.SelectStockObject(BLACK_PEN);
	dc.SelectStockObject(WHITE_BRUSH);
}
//-------------------------------------------------------------------------------------------------
CString GetRTClassName(HWND hWnd)
{
	CString Name= CWnd::FromHandlePermanent(hWnd)->GetRuntimeClass()->m_lpszClassName;
	return Name;
}
//-------------------------------------------------------------------------------------------------
int GetInt(CString FileName, CString SectionName, CString ParamName)
{
	char Buf[30];
	int BufSize= 29;

	GetPrivateProfileString(LPCSTR(SectionName),
                            LPCSTR(ParamName),
							"0",
                            Buf,
                            BufSize,
                            LPCSTR(FileName));

	int Param= atoi(Buf);
	return Param;
}

double GetDouble(CString FileName, CString SectionName, CString ParamName)
{
	char Buf[30];
	int BufSize= 29;

	GetPrivateProfileString(LPCSTR(SectionName),
                            LPCSTR(ParamName),
							"0",
                            Buf,
                            BufSize,
                            LPCSTR(FileName));

	double Param= atof(Buf);
	return Param;
}

void MakeLoopMessage()
{
  MSG msg;
  while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
    if (!AfxGetApp()->PumpMessage())
      {
      ::PostQuitMessage(0);
      return;
      }
    }
  AfxGetApp()->OnIdle(0);
  AfxGetApp()->OnIdle(1);
}

BOOL IsFileExist(LPCTSTR fName, BOOL needMessage/*true*/)
{
  int mode=0;
  CString mes;
  if(!_access(fName, mode)){
    return TRUE;
  }
  else if(needMessage){
    mes = CString(fName) + CString("\r\nfile is not available");
    AfxMessageBox(LPCTSTR(mes), MB_OK | MB_ICONEXCLAMATION);
  }
   return FALSE;
}

BOOL SavePath(LPCTSTR Key, LPCTSTR Str, LPCTSTR File)
{
   return ::WritePrivateProfileString("FILE_PATH", Key, Str, File);
}

BOOL ReadPath(LPCTSTR Key, LPTSTR Str, LPCTSTR File)
{
   LPCTSTR Def = NULL;
   if(GetPrivateProfileString("FILE_PATH", Key, Def, Str, sizeof(TCHAR)*_MAX_PATH, File))
      return TRUE;
   else
      return FALSE;
}

BOOL SaveSection(LPCTSTR Section, LPCTSTR Key, LPCTSTR Str, LPCTSTR File)
{
   return ::WritePrivateProfileString(Section, Key, Str, File);
}

BOOL ReadSection(LPCTSTR Section, LPCTSTR Key, LPTSTR Str, LPCTSTR File)
{
   LPCTSTR Def = "no data";
   GetPrivateProfileString(Section, Key, Def, Str, sizeof(TCHAR)*_MAX_PATH, File);
   if (CString(Str) == "no data") return FALSE;
   else return TRUE;
}

void GetAvaibleDrives(int& nDr, LPTSTR drives)
{
   int drive, curdrive, n;
   /* Save current drive. */
   curdrive = _getdrive();
   n=0;
   /* If we can switch to the drive, it exists. */
   for( drive = 3; drive <= 26; drive++ ) { //1
      if( !_chdrive( drive ) ){
         drives[n++] = (char)(drive + 'A' - 1);
      }
   }
   /* Restore original drive.*/
   _chdrive( curdrive );
   nDr = n;
}

void CheckLastError()
{
	DWORD lastEr = GetLastError();
	if(!lastEr)
		return;

	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		lastEr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}

BOOL CreateTMPUniqueFile(CString& name, LPCTSTR prefix)
{
  TCHAR _path[MAX_PATH];
  TCHAR _name[MAX_PATH];

  GetTempPath(MAX_PATH, _path);

  GetTempFileName(_path, prefix, 0, _name);
  name = _name;

  return TRUE;
}

BOOL WriteStringToFile(LPCTSTR str, LPCTSTR filepath, UINT nOpenFlags/*CFile::modeCreate |CFile::modeWrite*/)
{
  CStdioFile stdf;
  CFileException ex;

  if(!stdf.Open(filepath, nOpenFlags, &ex)){
      TCHAR szError[1024];
      ex.GetErrorMessage(szError, 1024);
	  AfxMessageBox(szError);
	  return FALSE;
  }
  stdf.WriteString(str);
  stdf.Close();

  return TRUE;
}

void CreateEllipseArray(CRect* pR, CPoint* aP, int naP)
{
  double x, x2;
  double y;
      double a = (double)pR->Width()/2.;
      double b = (double)pR->Height()/2.;
      double d = a /(naP-1);
    CPoint cent = pR->CenterPoint();
    for(int i=0; i < naP; i++){
       x = (double)pR->left -(double)cent.x + i*d;
       y = (1. - x*x/a/a)*b*b;
       if(!i) y = 0.;
       else y = sqrt(fabs(y));
       aP[i] = CPoint((int)x+cent.x,(int)y+cent.y);
       x2 = (double)pR->right -(double)cent.x - i*d;
       aP[naP*2-i-1] = CPoint((int)x2+cent.x,(int)y+cent.y);
       aP[naP*2+i] = CPoint((int)x2+cent.x,(int)-y+cent.y);
       aP[naP*4-i-1] = CPoint((int)x+cent.x,(int)-y+cent.y);
      }
}

void InflateContour(CPoint* pwLine, int nP, int val)
{
	int tol = abs(val);
	CRgn Rgn;
	BOOL res = Rgn.CreatePolygonRgn(pwLine, nP, WINDING);
	if(!res)
		return;
	CPoint curP, P1, P2, newP;
	BOOL insP1, insP2;
    for(int i=0; i < nP; i++){
	   curP = newP = pwLine[i];
	   P1.x = curP.x - tol; P1.y = curP.y;
	   P2.x = curP.x + tol; P2.y = curP.y;
	   insP1 = Rgn.PtInRegion(P1);
	   insP2 = Rgn.PtInRegion(P2);
	   if(val > 0 && !insP1 && insP2)
		  newP.x = curP.x + tol;
	   else if(val > 0 && insP1 && !insP2)
		  newP.x = curP.x - tol;
	   else if(val < 0 && !insP1 && insP2)
		  newP.x = curP.x - tol;
	   else if(val < 0 && insP1 && !insP2)
		  newP.x = curP.x + tol;

	   P1.y = curP.y - tol; P1.x = curP.x;
	   P2.y = curP.y + tol; P2.x = curP.x;
	   insP1 = Rgn.PtInRegion(P1);
	   insP2 = Rgn.PtInRegion(P2);
	   if(val > 0 && !insP1 && insP2)
		  newP.y = curP.y + tol;
	   else if(val > 0 && insP1 && !insP2)
		  newP.y = curP.y - tol;
	   else if(val < 0 && !insP1 && insP2)
		  newP.y = curP.y - tol;
	   else if(val < 0 && insP1 && !insP2)
		  newP.y = curP.y + tol;

	   pwLine[i] = newP;
	   }
}
