#if !defined(AFX_DOT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_DOT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_
//C:\Ilya\Programming\cpp\Numbering\DigitMode\DotInfo.h
#include "MGTools\StdAfx.h"
#include "Appdef.h"
#include "MGTools\Include\Utils\BaseDataType.h"

class CDotInfo : public CObject
{
public:
	int iZapSec; // TODO: remove - it's deprecated
	double Number;
	int segIdx; // Segment index to separate fringes with the same number
	CDPoint P;
public:
	CDotInfo();
	~CDotInfo();
	void Init();
	void Draw(CDC* pDC, int dotSide, BOOL mainDot = FALSE);

	CDotInfo(const CDotInfo& rhs) {
		{ operator=(rhs); }
	}

	CDotInfo& operator=(const CDotInfo& rhs);
};

#endif // !defined(AFX_DOT_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)

