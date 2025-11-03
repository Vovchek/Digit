#if !defined(AFX_ZAPLINE_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_ZAPLINE_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_

#include "stdafx.h"
#include "Appdef.h"
//C:\Ilya\Programming\cpp\Numbering\DigitMode\ZapLineInfo.h
#include "MGTools\Include\Utils\BaseDataType.h"

class CZapLineInfo : public CObject
{
   public:
	  int iSec;
	  double Number;
	  CDLine L;
	  BOOL Removed;
	  
   public:
	CZapLineInfo();
	~CZapLineInfo();
	void Init();
	void Draw(CDC* pDC);
	
    CZapLineInfo(const CZapLineInfo& rhs){
      { operator=(rhs);}
     }

    CZapLineInfo& operator=(const CZapLineInfo& rhs);
};

#endif // !defined(AFX_ZAPLINE_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)

