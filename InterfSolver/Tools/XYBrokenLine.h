#ifndef _XYBROKENLINE_H_
#define _XYBROKENLINE_H_

#include "include\CArrDef.h"
#include "XYPoint.h"

class XYBrokenLine
  {
  protected:
    CArray<XYPoint,XYPoint> ArrPnt;

  public:
    XYBrokenLine ();
    XYBrokenLine (int Size) {ArrPnt.SetSize(Size);}
    XYBrokenLine (const XYBrokenLine &A);
    XYBrokenLine (int NPnt, double *pArrX, double *pArrY);
    XYBrokenLine (int NPnt, int *pArrX, int *pArrY);
    XYBrokenLine (const CArrayDouble &ArrX, const CArrayDouble &ArrY);
    XYBrokenLine (const CArrayDouble &ArrXY);
    ~XYBrokenLine ();
    int GetSize() const {return ArrPnt.GetSize();}
    void SetSize (int Size) {ArrPnt.SetSize(Size);}
    void RemoveAt (int iElm) {ArrPnt.RemoveAt(iElm);}
    void RemoveAll () {ArrPnt.RemoveAll();}
    void Add (const XYPoint &P) {ArrPnt.Add(P);}
    void InsertAt(int Ind, XYPoint P, int Count = 1) {ArrPnt.InsertAt(Ind, P, Count);}
    void InsertAt(int StartInd, XYBrokenLine &A) 
                                            {ArrPnt.InsertAt(StartInd, &A.ArrPnt);}
    void Append(const XYBrokenLine &A) {ArrPnt.Append(A.ArrPnt);}
    XYBrokenLine& operator= (const XYBrokenLine &A);
    XYBrokenLine& operator= (const CArrayXYPoint &A);
    const XYPoint operator[] (int iEl) const {return ArrPnt[iEl];}
    XYPoint &operator[] (int iEl) {return ArrPnt[iEl];}
    void GetArrX(CArrayDouble &ArrX);
    void GetArrY(CArrayDouble &ArrY);
    void Inverse();
    bool FindNearPoint (const XYPoint P, int &iNear, double &Distn);
    XYPoint FindNearPoint (const XYPoint P);
    void Arrange();
    friend bool FindNearPoint(const XYPoint P, const XYBrokenLine &BLine, int &iNear,
                                                                               double &Distn);
    friend void Arrange(const XYBrokenLine &BLine);
  };
#endif
