#include <math.h>
#include <functional>
#include "InterfSolver\Include\Int_Cons.h"
#include "CalcContour.h"

/**
 * @brief Connects broken line segments into continuous contour polygons
 * 
 * This function takes an array of broken line segments (visible portions of shape boundaries)
 * and connects them by matching endpoints within a tolerance distance. The algorithm uses a
 * greedy approach to assemble fragments into complete closed contours.
 * 
 * @param ArrBLn Input array of broken line segments to connect
 * @param ArrCont Output array of resulting contour polygons
 * @param Eps Distance tolerance for considering endpoints "close enough" to connect
 * 
 * Algorithm:
 * 1. Take first segment from ArrBLn as starting contour
 * 2. Repeatedly search remaining segments for endpoints matching current contour endpoints
 * 3. Connect matching segments (reversing if needed to maintain direction)
 * 4. Close contour if endpoints are within tolerance
 * 5. Convert to polygon and add to output array
 * 6. Repeat until all segments are processed
 * 
 * Connection cases:
 * - New segment's start matches current contour's start → Reverse new segment, prepend
 * - New segment's end matches current contour's start → Prepend as-is
 * - New segment's start matches current contour's end → Append as-is
 * - New segment's end matches current contour's end → Reverse new segment, append
 */
void ConnectSegments(const CArrayXYBrokenLine& InputBLn, CArrayXYPolygon& ArrCont, double Eps)
{
  ArrCont.RemoveAll();
  auto NBLn = InputBLn.GetSize();
  
  if (NBLn == 0)
    return;
    
  if (NBLn == 1)
    {
    auto Plg = XYPolygon(InputBLn[0]);
    if(!Plg.isDegenerate())
        ArrCont.Add(Plg);
    return;
    }

  // Make a working copy since we'll be removing elements
  // Skip 0-length lines to avoid crash
  CArrayXYBrokenLine ArrBLn;
  for(auto i = 0; i < InputBLn.GetSize(); ++i) 
    {
      if (InputBLn[i].GetSize() > 0)
          ArrBLn.Add(InputBLn[i]);
    }
  
  while ((NBLn = ArrBLn.GetSize()) > 0)
    {
    auto CurCont = ArrBLn[0];
    ArrBLn.RemoveAt(0);
    auto NCur = CurCont.GetSize();
    auto Pn = CurCont[0];
    auto Pk = CurCont[NCur-1];
    bool isFound = true;
    
    while (isFound)
      {
      NBLn = ArrBLn.GetSize();
      if (NBLn == 0)
        break;
        
      for (auto iBLn = 0; iBLn < NBLn; iBLn++)
        {
        auto NPnt = ArrBLn[iBLn].GetSize();
        if (Distance(Pn, ArrBLn[iBLn][0]) < Eps)
          {
          ArrBLn[iBLn].Inverse();
          CurCont.InsertAt(0, ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          Pn = CurCont[0];
          isFound = true;
          break;
          }
        else if (Distance(Pn, ArrBLn[iBLn][NPnt-1]) < Eps)
          {
          CurCont.InsertAt(0, ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          Pn = CurCont[0];
          isFound = true;
          break;
          }
        else if (Distance(Pk, ArrBLn[iBLn][0]) < Eps)
          {
          CurCont.Append(ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          NCur = CurCont.GetSize();
          Pk = CurCont[NCur-1];
          isFound = true;
          break;
          }
        else if (Distance(Pk, ArrBLn[iBLn][NPnt-1]) < Eps)
          {
          ArrBLn[iBLn].Inverse();
          CurCont.Append(ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          NCur = CurCont.GetSize();
          Pk = CurCont[NCur-1];
          isFound = true;
          break;
          }
        isFound = false;
        }
      }
      
    if (CurCont.GetSize() > 0)
      {
      NCur = CurCont.GetSize();
      double closingDist = Distance(CurCont[0], CurCont[NCur-1]);
      
      if (closingDist < Eps * 2.0)
        {
        CurCont.Add(CurCont[0]);
        }
      
      auto Plg = XYPolygon(CurCont);
	  if (!Plg.isDegenerate())
        ArrCont.Add(Plg);
      }
    }
}

/**
 * @brief Computes visible contours of geometric shapes accounting for occlusion
 * 
 * This function calculates the visible boundary segments of geometric objects (ellipses,
 * rectangles, and polygons) by finding which parts of their boundaries are not hidden
 * by other shapes, then assembles them into complete contour polygons.
 * 
 * @param ArrEll Array of ellipses to process
 * @param ArrRect Array of rectangles to process  
 * @param ArrPlg Array of polygons to process
 * @param ArrCont Output array of resulting contour polygons
 * @param NPntMax Maximum number of points for contour discretization
 * 
 * Algorithm steps:
 * 1. Setup: Calculate step size based on maximum perimeter among all shapes
 * 2. Contour extraction: For each shape type, extract visible boundary segments
 *    using isPupil() to test visibility (not inside other shapes)
 * 3. Segment connection: Connect broken line segments into continuous contours
 *    using distance tolerance to match nearby endpoints
 * 4. Polygon classification: Mark contours as EXTERNAL or INTERNAL (holes)
 */
void CalcContour(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
                   const CArrayXYPolygon &ArrPlg, CArrayXYPolygon &ArrCont, int NPntMax)
  {
  ArrCont.RemoveAll();
  int iElm, iPnt, NPnt;
  CArrayXYBrokenLine ArrBLn;
  XYBrokenLine CurCont;
  XYBrokenLine CurBLn;
  XYPoint P;
 //------------------------------------------------------------------------
  // Setup and step size calculation
  int NEll = ArrEll.GetSize();
  int NRect = ArrRect.GetSize();
  int NPlg = ArrPlg.GetSize();
 //------------------------------------------------------------------------
  // Find maximum perimeter to determine point spacing
  double CurPerim, MaxPerim = 0., Step;
  for (iElm = 0; iElm < NEll; iElm++)
    {
    if ((CurPerim = ArrEll[iElm].Perimeter()) > MaxPerim)
      MaxPerim = CurPerim;
    }
  for (iElm = 0; iElm < NRect; iElm++)
    {
    if ((CurPerim = ArrRect[iElm].Perimeter()) > MaxPerim)
	      MaxPerim = CurPerim;
    }
  Step = MaxPerim / NPntMax;
 //------------------------------------------------------------------------
  // Lambda to extract visible segments from a contour
  auto extractVisibleSegments = [&](const XYBrokenLine& contour, 
                                     std::function<bool(const XYPoint&)> isVisible)
    {
    NPnt = contour.GetSize();
    CurBLn.RemoveAll();
    for (iPnt = 0; iPnt < NPnt; iPnt++)
      {
      P = contour[iPnt];
      if (isVisible(P))
        CurBLn.Add(P);
      else if (CurBLn.GetSize() > 0)
        {
        ArrBLn.Add(CurBLn);
        CurBLn.RemoveAll();
        }
      }
    if (CurBLn.GetSize() > 0)
      {
      ArrBLn.Add(CurBLn);
      CurBLn.RemoveAll();
      }
    };
 //------------------------------------------------------------------------
  // Extract visible contour segments from ellipses
  for (iElm = 0; iElm < NEll; iElm++)
    {
    ArrEll[iElm].GetContour(CurCont, Step);
    extractVisibleSegments(CurCont, [&](const XYPoint& pt) {
      return isPupil(pt, ArrEll, iElm) && isPupil(pt, ArrRect) && isPupil(pt, ArrPlg);
    });
    }
 //------------------------------------------------------------------------
  // Extract visible contour segments from rectangles
  for (iElm = 0; iElm < NRect; iElm++)
    {
    ArrRect[iElm].GetContour(CurCont, Step);
    extractVisibleSegments(CurCont, [&](const XYPoint& pt) {
      return isPupil(pt, ArrEll) && isPupil(pt, ArrRect, iElm) && isPupil(pt, ArrPlg);
    });
    }
 //------------------------------------------------------------------------
  // Extract visible contour segments from polygons
  for (iElm = 0; iElm < NPlg; iElm++)
    {
    CurCont = ArrPlg[iElm];
    NPnt = CurCont.GetSize();
    CurCont.RemoveAt(NPnt - 1);
    NPnt--;
    extractVisibleSegments(CurCont, [&](const XYPoint& pt) {
      return isPupil(pt, ArrEll) && isPupil(pt, ArrRect) && isPupil(pt, ArrPlg, iElm);
    });
    }
 //------------------------------------------------------------------------
  // Connect broken line segments into continuous contours
  double Eps = max(2.5 * Step, 1e-5);
  ConnectSegments(ArrBLn, ArrCont, Eps);
 //------------------------------------------------------------------------
  // Classify contours as external or internal (holes)
  int NCont = ArrCont.GetSize();
  if (NCont == 1)
    {
    ArrCont[0].SetTypeLimits(EXTERNAL);
    return;
    }
  // Mark all contours that aren't inside others as EXTERNAL
  int i;
  bool isInsideAny;
  for (iElm = 0; iElm < NCont; iElm++)
    {
    P = ArrCont[iElm][0];
    isInsideAny = false;
    for (i = 0; i < NCont; i++)
      {
      if (i == iElm)
        continue;
      if (ArrCont[i].isInside(P))
        {
        isInsideAny = true;
        break;
        }
      }
    ArrCont[iElm].SetTypeLimits(isInsideAny ? INTERNAL : EXTERNAL);
    }
  }
//=========================================================================
