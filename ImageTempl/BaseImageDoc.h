#if !defined(AFX_BASEIMAGEDOC_H__D5386BEE_009B_4643_A333_F9F54446D0A0__INCLUDED_)
#define AFX_BASEIMAGEDOC_H__D5386BEE_009B_4643_A333_F9F54446D0A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BaseImageDoc.h : header file
//
#include "Controls\ImageCtrls.h"
#include "Controls\MeasureCtrls.h"
#include "Controls\BoundCtrls.h"
#include "Controls\CApertureCtrls.h"  // ← Modern aperture controls
#include "DigitMode\DigitInfo.h"
#include "Utils\Tracker.h"

class CBaseImageView;
class CImageCtrls;
class CMeasureCtrls;
//class CBoundCtrls;
class CDigitInfo;

namespace DigitMode {
    class CApertureCtrls;
}
/////////////////////////////////////////////////////////////////////////////
// CBaseImageDoc document
class CBaseImageDoc : public CDocument
{
protected:
	CBaseImageDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBaseImageDoc)

// Attributes
public:
   int LastOperationType;
   CImageCtrls imageCtrls;
   CMeasureCtrls measureCtrls;
   //CBoundCtrls boundCtrls;                      // ← Legacy bounds (to be deprecated)
   DigitMode::CApertureCtrls apertureCtrls;     // ← Modern aperture system
   CMTraker  Tracker;
   CDigitInfo Digit;
   CStringList Scenario;
   CString TmpPath;
   CString UndoTmpPath;
   
// Operations
public:
    CBaseImageView* GetView();
	void InitData();
    BOOL ReloadDocument(LPCTSTR lpszImagePathName); /** @brief Reloads the document with the specified image file. */
	LPCTSTR GetTmpPath();
    LPCTSTR GetUndoTmpPath();
	LPCTSTR GetRealPath();
	LPCTSTR GetRealName();
	void SetRealPath(LPCTSTR _RealPath);
	void SetRealName(LPCTSTR _RealName);
	void CopyImage();

	void AddRecordScenario(int idCom, LPCTSTR _Options);
	void RemoveLastRecordScenario();
	void SetScenario(CStringList& _Scenario);
	void GetScenario(CStringList& _Scenario);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseImageDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBaseImageDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  CString RealPath;
  CString RealName;

// Generated message map functions
protected:
	//{{AFX_MSG(CBaseImageDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
    afx_msg void OnFileClose();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEIMAGEDOC_H__D5386BEE_009B_4643_A333_F9F54446D0A0__INCLUDED_)
