#if !defined(AFX_IMAGEDOC_H__8056A041_1BE0_4DEB_A9DE_546A3848E6B4__INCLUDED_)
#define AFX_IMAGEDOC_H__8056A041_1BE0_4DEB_A9DE_546A3848E6B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageDoc.h : header file
//
#include "ImageTempl\BaseImageDoc.h" 
#include "ImageTempl\BaseImageView.h" 
#include "InterfSolver\Numbering\CalcAprImageMatr.h" 


class CSectionFrame;
class CBaseImageDoc;
class CImageView;
/////////////////////////////////////////////////////////////////////////////
// CImageDoc document

class CImageDoc : public CBaseImageDoc
{
protected:
	CImageDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CImageDoc)

// Attributes
public:
    CSectionFrame* pHorz;
    CSectionFrame* pVert;
	int LoadedFileType;
	Matrix MApr;
    int nx;
    int ny;
    int nLevel;
    double* pLevel;
    double* pX;
    double* pY;
    double* pMatr;
    double minZ, maxZ;
    BOOL IsAproxMatrix;
// Operations
public:
   void SetZoomToTitle();
   CImageView* GetView();
   BOOL ReloadDocument(LPCTSTR lpszPathName=NULL);
   void SetTextWndInfo();
   void GetImageInfo(CString& Info);
   void PrepareOperationUndo(int OperType);
   void LastOperationUndo();
   void DeActivateAllMode();
   void OnActivate();
   
   void ActivateMeasure(BOOL key);
   void ActivateExtBounds(BOOL key);
   void ActivateInsBounds(BOOL key);
   void WriteMeasureCtrls();
   bool IsFotoSections();
   void ActivateFotoSections(BOOL key);
   void AlignFotoSections(BOOL ActivateImage=TRUE);
   void ReSetSections(CPoint P = CPoint(-1,-1));
   void ReSetHorzSection(CPoint P = CPoint(-1,-1));
   void ReSetVertSection(CPoint P = CPoint(-1,-1));

   void AutoDigit();
   void ClearDigit();
   void OnNumberMinus();
   void OnNumberPlus();
   void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   void OnLButDown(CPoint P);
   void OnRButDown(CPoint P);
   void SelectMainDot(CPoint P);
   void SelectMainDot(int iSec, double Number);
   void AddDot(CPoint P);
   void RemoveDot(CPoint P);
   void RemoveFringe(CPoint P);
   void AddZapSection(int iy);
   void DeleteZapSection(int iy);
   void RenumFringe(CPoint P);
   void RenumDot(CPoint P);

   bool IsInterferogram();
   bool IsSections();
   bool IsDots();
   BOOL IsAproximation();
   bool IsDotUnderCursor(CPoint P);
   bool IsLockedDot();
   bool LockDot(CPoint P, BOOL Enable);
   void SetLockedDotPos(CPoint P);
   void GetLockedDotPos(CPoint& P1);
   void SectionLeft(CPoint P);
   void SectionRight(CPoint P);
   bool IsZapSections();
   bool IsLockedZapSection();
   bool IsZapSectionUnderCursor(CPoint P);
   bool LockZapSection(CPoint P, BOOL Enable);
   void SetLockedZapSectionYPos(int iy);
   void GetLockedZapSectionXYPos(CPoint& P1, CPoint& P2);

   BOOL CheckForSave();

   void ActivateAproximation();
   void CalcAproximation();
   void CreateAproxImage(CArrayDouble& ArrFNumbers);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void DeleteContents();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual void SetTitle(LPCTSTR lpszTitle);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	
	//{{AFX_MSG(CImageDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEDOC_H__8056A041_1BE0_4DEB_A9DE_546A3848E6B4__INCLUDED_)
