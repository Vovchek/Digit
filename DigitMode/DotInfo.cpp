#include "DotInfo.h"

#include "Utils\mutils.h"
#include "Utils\middle.h"
#include "MGTools\Include\Utils\Utils.h"

CDotInfo::CDotInfo()
{
	Init();
}

CDotInfo::~CDotInfo()
{
}

void CDotInfo::Init()
{
  iZapSec = -1;
  Number = -1000.;
  P.x = P.y = 0.;
}

CDotInfo& CDotInfo::operator=(const CDotInfo& rhs)
{
  if(this==&rhs) return *this;
    iZapSec = rhs.iZapSec;
    Number = rhs.Number;
    P = rhs.P;
  return *this;
}

void CDotInfo::Draw(CDC* pDC, int dotSide, BOOL mainDot/*FALSE*/)
{
   CControls* pCtrls = GetControls();
   int DotSide12;
   COLORREF Color;
   if(mainDot){
	   Color = RGB(255,0,0);
       DotSide12 = (int)(dotSide*0.5+0.5)+1;
   }
   else{
      pCtrls->GetIndexColor(Number, Color);
      DotSide12 = (int)(dotSide*0.5+0.5);
   }
   CPoint lP;
   lP.x = (int)P.x;
   lP.y = (int)P.y;
   CRect dotR;
   dotR.left = lP.x - DotSide12; 
   dotR.right = lP.x + DotSide12; 
   dotR.top = lP.y - DotSide12; 
   dotR.bottom = lP.y + DotSide12; 
   CPen pen;
   pen.CreatePen(PS_SOLID, 0, Color);
   CPen* open = pDC->SelectObject(&pen);
   CBrush br;
   CBrush* obr;
   br.CreateSolidBrush(Color);

   obr = pDC->SelectObject(&br);
   pDC->Ellipse(&dotR);

   CPen* retPen = pDC->SelectObject(open);
   if(retPen)
      retPen->DeleteObject();
   CBrush* retBr = pDC->SelectObject(obr);
   if(retBr)
      retBr->DeleteObject();
}


