#include "SectionInfo.h"

#include "Utils\mutils.h"
#include "Utils\middle.h"
#include "MGTools\Include\Utils\Utils.h"

//====================  CNumLine ============================
CNumLine::CNumLine()
{
	Init();
}

CNumLine::~CNumLine()
{
}

void CNumLine::Init()
{
   Number = -1000.;
   segmL.Init();
   Included = FALSE;
   redX = 0.;
}

CNumLine& CNumLine::operator=(const CNumLine& rhs)
{
  if(this==&rhs) return *this;
   Number = rhs.Number;
   segmL = rhs.segmL;
   redX = rhs.redX;
   Included = rhs.Included;
  return *this;
}

//====================  CSectionInfo ============================
CSectionInfo::CSectionInfo()
{
	Init();
}

CSectionInfo::~CSectionInfo()
{
}

CSectionInfo& CSectionInfo::operator=(const CSectionInfo& rhs)
{
  if(this==&rhs) return *this;
	 L = rhs.L;
	 Color = rhs.Color;
	 MainLine = rhs.MainLine;
	 VisibleRedDots = rhs.VisibleRedDots;
	 NumLines.RemoveAll();
     for(int i=0; i < rhs.NumLines.GetSize(); i++){
       NumLines.Add(rhs.NumLines[i]);
	 }
    return *this;
}

void CSectionInfo::Init()
{
	Color = RGB(255, 255, 255);
	MainLine = FALSE;
	VisibleRedDots = TRUE;
	aveStep = -1.;
}

void CSectionInfo::Draw(CDC* pDC, int MainFringeNumber)
{
   CControls* pCtrls = GetControls();
   COLORREF Color;
   int x, y; 
// Main Section   
/*		y = (int)L.P1.y;
		if(MainLine){
			CPoint P1, P2;
			P1.y = P2.y = y;
			P1.x = (int)L.P1.x;
			P2.x = (int)L.P2.x;
			pDC->MoveTo(P1);
			pDC->LineTo(P2);
		}
*/		
//    Color Segments		
/*		for(int iD=0; iD < NumLines.GetSize(); iD++){
			if(!NumLines[iD].Included)
				continue;
			if(NumLines[iD].Number == MainFringeNumber)
				Color = RGB(255,0,0);
			else
			  pCtrls->GetIndexColor(NumLines[iD].Number, Color);
            CPen pen;
            pen.CreatePen(PS_SOLID, 1, Color);
            CPen* open = pDC->SelectObject(&pen);
			CPoint P1, P2;
			P1.y = P2.y = y;
			P1.x = (int)NumLines[iD].segmL.P1.x;
			P2.x = (int)NumLines[iD].segmL.P2.x;
			pDC->MoveTo(P1);
			pDC->LineTo(P2);
		}*/
		Color = RGB(255,0,0);
		for(int iD=0; iD < NumLines.GetSize(); iD++){
			if(!NumLines[iD].Included)
				continue;
          x = (int)NumLines[iD].redX;
   		  y = (int)L.P1.y;
		  pDC->SetPixelV(x, y, Color);
		}
}

void CSectionInfo::Sort()
{
    int swapAt;
	int begIdx = 0;
	int endIdx = NumLines.GetSize();
    double d_swapAt; 
    double d_k;
    for(int j=begIdx; j < endIdx-1; j++){
      swapAt = j;
      for(int k=j+1; k < endIdx; k++){
        d_swapAt = NumLines[swapAt].redX;
        d_k = NumLines[k].redX;
        if(d_swapAt > d_k)
            swapAt = k;
      }
      Swap(NumLines[j], NumLines[swapAt]);
    }
}

void CSectionInfo::CalcAveStep()
{
    int	nD = NumLines.GetSize();
	if(nD>2){
 	  double xMin = NumLines[0].redX;
 	  double xMax = NumLines[NumLines.GetSize()-1].redX;
	  aveStep = (xMax - xMin)/(nD-1);
	}
}
