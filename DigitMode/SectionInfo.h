#if !defined(AFX_SECTION_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_SECTION_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_

#include "MGTools\StdAfx.h"
#include "Appdef.h"
#include "MGTools\Include\Utils\BaseDataType.h"

class CNumLine : public CObject
{
  public:	
	double Number;
	CDLine segmL;
	BOOL Included;
	double redX;

  public:
	CNumLine();
	~CNumLine();
	void Init();
	
    CNumLine(const CNumLine& rhs){
      { operator=(rhs);}
     }

    CNumLine& operator=(const CNumLine& rhs);
};

class CSectionInfo : public CObject
{
   public: 
     CDLine L;
	 CArray<CNumLine, CNumLine> NumLines;
	 double aveStep;
	 COLORREF Color;
	 BOOL MainLine;
	 BOOL VisibleRedDots;

   public: 
    CSectionInfo();
    ~CSectionInfo();
	void Init();
	void Draw(CDC* pDC, int MainFringeNumber);
	void Sort();
    bool Form(int iy, unsigned char* line, int nx, int ny, int **buf_line);
    void CalcAveStep();
	
    CSectionInfo(const CSectionInfo& rhs){
      { operator=(rhs);}
     }

    CSectionInfo& operator=(const CSectionInfo& rhs);
};

#endif // !defined(AFX_SECTION_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
