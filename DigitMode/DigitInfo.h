#if !defined(AFX_DIGIT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_DIGIT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_
//C:\Ilya\Programming\cpp\Numbering\DigitMode\DigitInfo.h
#include "MGTools\StdAfx.h"
#include "Appdef.h"

#include "MGTools\Include\Utils\BaseDataType.h"

#include "DigitMode\SectionInfo.h"
#include "DigitMode\DotInfo.h"
#include "DigitMode\ZapLineInfo.h"
#include "DigitMode\CFringe.h"  // NEW: Fringe-based model

#include "InterfSolver\Tools\ReadWriteData.h"

class CImageCtrls;  // Forward declaration

/// <summary>
/// Selection point structure for fringe-based model.
/// Identifies a specific point within a fringe using (iFringe, iPoint) coordinates.
/// </summary>
struct SelectedPoint {
    int iFringe;  ///< Index into Fringes array
    int iPoint;   ///< Index into Points array within fringe
    
    SelectedPoint() : iFringe(-1), iPoint(-1) {}
    SelectedPoint(int f, int p) : iFringe(f), iPoint(p) {}
    
    BOOL IsValid() const { return iFringe >= 0 && iPoint >= 0; }
    void Clear() { iFringe = iPoint = -1; }
    
    BOOL operator==(const SelectedPoint& other) const {
        return iFringe == other.iFringe && iPoint == other.iPoint;
    }
};

class CDigitInfo
{
  public:
   int **buf_line;
   int ny_buf_line;
   CArray<CSectionInfo> Sections;
   CArray<CZapLineInfo> ZapLines;
   
   // ===== OLD: Flat array model (keep during transition) =====
   CArray<CDotInfo> Dots;
   int idxDragDot;
   int idxMainDot;
   
   // ===== NEW: Fringe-based model =====
   CArray<CFringe> Fringes;
   BOOL m_bUseFringeModel;  ///< Transition flag (default FALSE)
   SelectedPoint idxDraggedPoint;  ///< Replaces idxDragDot in fringe model
   SelectedPoint idxMainPoint;     ///< Replaces idxMainDot in fringe model
   
   // ===== Common properties =====
   BOOL HandSetZapLines;
   int idxDragZapLine;
   int idxMainSection;
   double MainFringeNumber;
   double CurrentNumber;
   double SecSegm;
   double CorrectionSecSegm;
   double numStep;
   BOOL isInsideScreen;

   CArray<CDPoint> HidenDots;

   CString Comments;
   double ScaleFactor;
   double Rotation;
	  
  public:
	  CDigitInfo();
	  virtual ~CDigitInfo();
	  void Init();

	  void Init_buf_line(int ny, int n);
      void Delete_buf_line();
	  
	  BOOL IsDigiting();
	  void Auto();
	  void Clear(BOOL AllZAPSections=TRUE);
	  void AddDot(CPoint P, int dotSide);
	  void RemoveDot(CPoint P, int dotSide);
	  void RemoveFringe(CPoint P, int dotSide);
	  void RemoveDotZAPSection(int iSec);
      void AddZapSection(int iy);
      void DeleteZapSection(int iy);
  	  void RenumFringe(CPoint P, int dotSide);
  	  void RenumDot(CPoint P, int dotSide);
	  void NumberMinus();
	  void NumberPlus();

      void CreateBufLine();
      void CreateBufLineAperture();
      void CreateBufLineOnstruction();
	  void CreateBufLineApertureComplex();
	  void CreateBufLineApertureSimple();
	  void CreateBufLineObstructionComplex();
	  void CreateBufLineObstructionSimple();
	  void CreateRedCenters(); // TODO: remove UI dependency
	  void Draw(CDC* pDC, int DotSide); // TODO: remove UI dependency

	  void CalcSectionAveSteps();
	  void SelectFringeStep();
	  void SelectMainSection();

	  void CreateNumLines();
	  void NumberingLine();
	  void SelectMainFringe();
  	  void CorrectNumbers();
      bool SelectNumber(int iSec, int Sign, double redX, int& idxS, int& idxL);

	  bool IsSections();
	  void CreateZAPSections();
	  void CreateZAPSectionsOnLoadZAPFile();
 	  void SortZapLines();  
      void SortDotsFY();
      void PutDotsOnZAPSections(int iZAPSec);

	  void SelectMainDot(CPoint P, int dotSide);
      void SelectMainDot(int iSec=-1, double Number=INT_MIN);
	  
      bool IsDots();
      bool IsLockedDot();
      bool LockDot(CPoint P, int dotSide, BOOL Enable);
      bool GetDot(int iZapSec, double Number, int& idx, CDPoint& dP);
      void SetLockedDotPos(CPoint P);
      void GetLockedDotPos(CPoint& P1);
      bool GetDotNumbers(CList<double, double>& Numbers);
	  bool GetFirstDotInSection(int iSec, int& idx, CDPoint& dP);
	  bool GetNextDotInSection(int iSec, int direct, int& idx, CDPoint& dP);
	  bool IsDotUnderCursor(CPoint P, int dotSide, int& idx);

      bool IsZapSections();
      bool IsLockedZapSection();
      bool IsZapSectionUnderCursor(CPoint P, int& idx);
	  bool LockZapSection(CPoint P, BOOL Enable);
      void SetLockedZapSectionYPos(int iy);
      void SectionLeft(CPoint P, int dotSide);
      void SectionRight(CPoint P, int dotSide);
      void GetLockedZapSectionXYPos(CPoint& P1, CPoint& P2);

      bool GetFringeDots(double Number, CUIntArray& idxDots);
      bool GetFringeDots(double Number, CArray<CDPoint>& adP);
	  bool GetFirstDotInFringe(double Number, int& idx, CDPoint& dP);
      bool GetNextDotInFringe(double Number, int direct, int& idx, CDPoint& dP);
      bool GetNearestZapSection(CPoint P, int& idx);
	  bool GetNearestXInSection(CPoint P, double& x);

	  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags); // TODO: remove UI dependency

	  void SetComments(LPCTSTR _Comments){Comments = _Comments;}
      void SetScaleFactor(double _ScaleFactor){ScaleFactor = _ScaleFactor;}
      void SetRotation(double _Rotation){Rotation = _Rotation;}
	  void GetComments(CString& _Comments){_Comments = Comments;}
      void GetScaleFactor(double& _ScaleFactor){_ScaleFactor = ScaleFactor;}
      void GetRotation(double& _Rotation){_Rotation = Rotation;}
      BOOL CollectNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO &IntInfo);
      BOOL ExamineNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO &IntInfo);
	  BOOL Load(LPCTSTR fname); // TODO: remove file dependency
	  BOOL Save(LPCTSTR fname, int extIdx); // TODO: remove file dependency
	  BOOL LoadZAP(LPCTSTR fname); // TODO: remove file dependency
	  BOOL LoadFRN(LPCTSTR fname); // TODO: remove file dependency
	  BOOL SaveZAP(LPCTSTR fname, int extIdx); // TODO: remove file dependency
	  BOOL SaveFRN(LPCTSTR fname); // TODO: remove file dependency

  // ===== NEW: Fringe-based interface =====
  public:
	  /// Create new fringe and return its index in Fringes array
	  int CreateFringe(double number, int segment = -1);
	  
	  /// Delete fringe by index in Fringes array
	  void DeleteFringe(int iFringe);
	  
	  /// Get fringe by index
	  CFringe* GetFringe(int i);
	  const CFringe* GetFringe(int i) const;
	  
	  /// Find all fringes with given number
	  void FindFringesByNumber(double number, CArray<int>& indices);
	  
	  /// Add point to end of fringe
	  void AddPointToFringe(int iFringe, CDPoint p);
	  
	  /// Insert point at specific position in fringe
	  void InsertPointInFringe(int iFringe, int iPoint, CDPoint p);
	  
	  /// Remove point from fringe
	  void RemovePointFromFringe(int iFringe, int iPoint);
	  
	  /// Move point within fringe
	  void MovePointInFringe(int iFringe, int iPoint, CDPoint newP);
	  
	  /// Find point under cursor (returns fringe and point indices)
	  BOOL FindPointUnderCursor(CPoint P, int tolerance, int& outFringe, int& outPoint);
	  
	  /// Find fringe under cursor
	  BOOL FindFringeUnderCursor(CPoint P, int tolerance, int& outFringe);
	  
	  /// Renumber all fringes with oldNumber to newNumber
	  void RenumberFringes(double oldNumber, double newNumber);
	  
	  /// Delete all fringes with given number
	  void DeleteFringesByNumber(double number);
	  
	  /// Merge two fringes (must have same number)
	  BOOL MergeFringes(int iFringe1, int iFringe2);
	  
	  // ===== Conversion utilities (Transition only) =====
	  
	  /// Convert Dots array to Fringes
	  void ConvertDotsToFringes();
	  
	  /// Convert Fringes to Dots array
	  void ConvertFringesToDots();
	  
	  /// Synchronize Fringes to Dots (maintains both models)
	  void SyncFringesToDots();

  protected:
	  void ProcessSectionPropagation(int sectionIndex, int direction, 
		  CArray<CNumLine>& refNumLines, int& maxSize, 
		  double& minN, double& maxN, double& leftX, double& rightX);
private:
    // Helper method to resolve relative image path to absolute path
    static std::string ResolveImagePath(const std::string& dataFilePath, const std::string& imageFileName);
    
    // Helper method to create a fake gray image when actual image is missing
    static BOOL CreateFakeGrayImage(CImageCtrls* pImageCtrls, int width, int height);
};
#endif // !defined(AFX_DIGIT_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
