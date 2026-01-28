
```mermaid

classDiagram
    CWinApp <|-- CDigitApp
    class CDigitApp {
      +CDigitApp()
      +BOOL InitInstance()
      +int ExitInstance()
      +BOOL PreTranslateMessage(MSG* pMsg)
      +BOOL OnIdle(LONG lCount)
      +CDocument* OpenDocumentFile(LPCTSTR lpszFileName)
      +BOOL FirstInstance(LPCTSTR CmdLine)
      +bool ChangeIntrLang(int iLang=-1)
      +void AddAdvancedTemplates()
      +bool TreatCmdLine(LPCTSTR CmdLine)
      +void OnFileOpen()
      +void OnAppAbout()
      -int intrLang
      -CString fINI
      -HINSTANCE rc_hInstance
      +static int m_nOpenMsg
    }
    CDigitApp *-- CControls : contains
    CDigitApp --> CMainFrame : creates/owns (m_pMainWnd)

    CMDIFrameWnd <|-- CMainFrame
    class CMainFrame {
      +BOOL LoadFrame(UINT nIDResource)
      +void SetImageInfo(CString comments, double scale, double rotation)
    }

    CMDIChildWnd <|-- CImageChildFrm
    class CImageChildFrm {
      +OnCreate()
    }

    CDialog <|-- CAboutDlg
    class CAboutDlg {
      +CAboutDlg()
      +DoDataExchange(CDataExchange* pDX)
    }

    CDialogBar <|-- CCommentsFile
    class CCommentsFile {
      +...
    }

    CDocument <|-- CBaseImageDoc
    class CBaseImageDoc {
      +virtual void Serialize(CArchive& ar)
      +...
    }

    CBaseImageDoc <|-- CImageDoc
    class CImageDoc {
      +CImageDoc()
      +BOOL ReloadDocument(LPCTSTR lpszPathName=NULL)
      +void GetImageInfo(CString& Info)
      +void OnActivate()
      +void AutoDigit()
      +void ClearDigit()
      +void Load(LPCTSTR fname)
      -int LoadedFileType
      -Matrix MApr
      -CDigitInfo Digit
    }
    CImageDoc *-- CDigitInfo : has

    SECZoomView <|-- CBaseImageView
    CBaseImageView <|-- CImageView
    class CImageView {
      +OnDraw(CDC* pDC)
      +...
    }

    class CDigitInfo {
      +CDigitInfo()
      +~CDigitInfo()
      +void Init()
      +void Auto()
      +void Clear(BOOL AllZAPSections=TRUE)
      +BOOL Load(LPCTSTR fname)
      +BOOL Save(LPCTSTR fname, int extIdx)
      +BOOL LoadZAP(LPCTSTR fname)
      +BOOL LoadFRN(LPCTSTR fname)
      +BOOL SaveZAP(LPCTSTR fname, int extIdx)
      +BOOL SaveFRN(LPCTSTR fname)
      +void CreateBufLine()
      +void CreateRedCenters()
      +void SelectFringeStep()
      +void SelectMainSection()
      +void CreateNumLines()
      +void CreateZAPSections()
      +void CreateZAPSectionsOnLoadZAPFile()
      +void Draw(CDC* pDC, int DotSide)
      +void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
      -int **buf_line
      -int ny_buf_line
      -CArray~CSectionInfo~ Sections
      -CArray~CZapLineInfo~ ZapLines
      -CArray~CDotInfo~ Dots
      -CArray~CDPoint~ HidenDots
      -CString Comments
      -double ScaleFactor
      -double Rotation
      -double numStep
      -double SecSegm
      -double CorrectionSecSegm
      -BOOL isInsideScreen
    }

    
    class CImageCtrls {
      +BOOL LoadImage(LPCTSTR path)
      +CSize ImageSize
      +CString OriginalPath
      +CDIB* m_pDIB
    }
    class CBoundCtrls {
      +BOOL GetExtCorBound(int type, int xDIB, int yDIB, CRect& outR, BOOL a, BOOL b)
      +BOOL GetInsCorBound(int type, int xDIB, int yDIB, CRect& outR, BOOL a, BOOL b)
      +void FormBoundsOnLoadFile()
      -CArray~XYEllipse~ ArrEll
      -CArray~XYRect~ ArrRect
      -CArray~XYPolygon~ ArrPlg
      -int ExtBoundType
      -int InsBoundType
    }
    class CControls {
      +void LoadINISettings()
      +int ViewState
      +int FringeCenterAs
      +void GetIndexColor(double, COLORREF&)
      +CString GetWorkPath()
    }

    CDigitInfo ..> CImageCtrls : uses
    CDigitInfo ..> CBoundCtrls : uses
    CDigitInfo ..> CControls : uses
    CImageDoc ..> CImageCtrls : accesses
    CImageDoc ..> CBoundCtrls : accesses

    class NUMBERING_INTERFEROGRAM_INFO {
      +CString Title
      +double ScaleFactor
      +double FiScan
      +CArray~XYEllipse~ ArrEll
      +CArray~XYRect~ ArrRect
      +CArray~XYPolygon~ ArrPlg
      +SAMPLE_DATA DigitDat
      +int ImageSize[2]
      +CString ImageFileName
      +int LoadedFileType
    }
    CDigitInfo ..> NUMBERING_INTERFEROGRAM_INFO : serializes/deserializes

    CDocument <|-- C3DGraphDoc
    class C3DGraphDoc {
      +...
    }
```
```mermaid

classDiagram
    class CSectionInfo {
      +CalcAveStep()
      +Form(int idx, unsigned char* line, int n, int ny, int** buf_line)
      +Sort()
      -CArray~CNumLine~ NumLines
      -CDLine L
      -double aveStep
    }

    class CDotInfo {
      +CDotInfo()
      +void Draw(CDC* pDC, int DotSide, COLORREF Color=RGB(0,0,0))
      -CDPoint P
      -double Number
      -int iZapSec
    }

    class CZapLineInfo {
      +CZapLineInfo()
      +void Draw(CDC* pDC)
      -CDLine L
      -int iSec
      -bool Removed
    }

    %% Примечание: вспомогательные/мелкие классы (CDIB, CNumLine, CDLine, CDPoint, SAMPLE_DATA и т.д.) опущены для краткости.