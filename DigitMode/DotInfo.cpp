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
  segIdx = -1;
  P.x = P.y = 0.;
}

CDotInfo& CDotInfo::operator=(const CDotInfo& rhs)
{
  if(this==&rhs) return *this;
    iZapSec = rhs.iZapSec;
    Number = rhs.Number;
	segIdx = rhs.segIdx;
    P = rhs.P;
  return *this;
}

void CDotInfo::Draw(CDC* pDC, int dotSide, BOOL mainDot/*FALSE*/)
{
   CControls* pCtrls = GetControls();
   int DotSide12;
   COLORREF Color;
   // Adjust dot size to be screen-constant: if a world transform is active on the DC,
   // convert desired screen size into world units by dividing by scale. If no transform,
   // dotSide already specified in world coords.
   double screenDot = dotSide; // pixels desired on screen
   // Try to query mapping mode: if world transform set, use it. Fallback to provided size.
   XFORM x;
   bool hasXform = false;
   if (GetWorldTransform(pDC->GetSafeHdc(), &x)) {
       hasXform = true;
   }
   if (hasXform) {
       // scale is approximately eM11 (assume uniform scale)
       double scale = x.eM11;
       // convert screen pixels to world units
       double worldSize = screenDot / (scale == 0.0 ? 1.0 : scale);
       DotSide12 = (int)(worldSize*0.5 + 0.5);
   }
   else {
       if(mainDot){
           DotSide12 = (int)(dotSide*0.5+0.5)+1;
       }
       else{
           DotSide12 = (int)(dotSide*0.5+0.5);
       }
   }
   if(mainDot){
       Color = RGB(255,0,0);
   }
   else{
       pCtrls->GetIndexColor(Number, Color);
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
