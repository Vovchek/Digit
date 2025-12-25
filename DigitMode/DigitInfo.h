#if !defined(AFX_DIGIT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_DIGIT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_
//C:\Ilya\Programming\cpp\Numbering\DigitMode\DigitInfo.h
#include "MGTools\StdAfx.h"
#include "Appdef.h"

#include "MGTools\Include\Utils\BaseDataType.h"

#include "DigitMode\SectionInfo.h"
#include "DigitMode\DotInfo.h"
#include "DigitMode\ZapLineInfo.h"

#include "InterfSolver\Tools\ReadWriteData.h"

class CDigitInfo
{
  public:
   int **buf_line;
   int ny_buf_line;
   CArray<CSectionInfo> Sections;
   CArray<CZapLineInfo> ZapLines;
   CArray<CDotInfo> Dots;
   BOOL HandSetZapLines;
   int idxDragZapLine;
   int idxDragDot;
   int idxMainSection;
   int idxMainDot;
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
	  void CreateRedCenters();
      void Draw(CDC* pDC, int DotSide);

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
      void SortDots(CArray<CDPoint>& adP, int XY);
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

	  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	  void SetComments(LPCTSTR _Comments){Comments = _Comments;}
      void SetScaleFactor(double _ScaleFactor){ScaleFactor = _ScaleFactor;}
      void SetRotation(double _Rotation){Rotation = _Rotation;}
	  void GetComments(CString& _Comments){_Comments = Comments;}
      void GetScaleFactor(double& _ScaleFactor){_ScaleFactor = ScaleFactor;}
      void GetRotation(double& _Rotation){_Rotation = Rotation;}
      BOOL CollectNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO &IntInfo);
      BOOL ExamineNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO &IntInfo);
	  BOOL Load(LPCTSTR fname);
	  BOOL Save(LPCTSTR fname, int extIdx);
	  BOOL LoadZAP(LPCTSTR fname);
	  BOOL LoadFRN(LPCTSTR fname);
	  BOOL SaveZAP(LPCTSTR fname, int extIdx);
	  BOOL SaveFRN(LPCTSTR fname);

  protected:
};
#endif // !defined(AFX_DIGIT_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
