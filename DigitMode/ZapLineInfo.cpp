#include "ZapLineInfo.h"

#include "Utils\mutils.h"
#include "Utils\middle.h"
#include "MGTools\Include\Utils\Utils.h"

CZapLineInfo::CZapLineInfo()
{
	Init();
}

CZapLineInfo::~CZapLineInfo()
{
}

void CZapLineInfo::Init()
{
  iSec = -1;
  Number = -1000.;
  L.Init();
  Removed = FALSE;
}

CZapLineInfo& CZapLineInfo::operator=(const CZapLineInfo& rhs)
{
  if(this==&rhs) return *this;
    iSec = rhs.iSec;
    Number = rhs.Number;
    L = rhs.L;
	Removed = rhs.Removed;
  return *this;
}

void CZapLineInfo::Draw(CDC* pDC)
{
	CPoint P1, P2;
	P1.y = P2.y = (int)L.P1.y;
	P1.x = (int)L.P1.x;
	P2.x = (int)L.P2.x;
	pDC->MoveTo(P1);
	pDC->LineTo(P2);
}



