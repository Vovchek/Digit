#ifndef __SAMPLDAT_H
#define __SAMPLDAT_H
#include "include\CArrDef.h"
struct SAMPLE_DATA
  {
  public:
    CArrayDouble  Properties;   // The sample properties;
    CArrayDouble  XPnt;         // The array X point coordinates of the sample
    CArrayDouble  YPnt;         // The array Y point coordinates of the sample
    CArrayDouble  FPnt;         // The array point function values of the sample
  public:
    SAMPLE_DATA ();
    SAMPLE_DATA& operator= (const SAMPLE_DATA &A);
    int GetSize();
    void SetSize(int NewSize);
    void Append(const SAMPLE_DATA &A);
    void Add(double X, double Y, double F, double P = 0.);
    void SortIncreaseX();
    void SortDecreaseX();
    void SortIncreaseY();
    void SortDecreaseY();
    void SortIncreaseF();
    void SortDecreaseF();
    void InverseY(double YcInv);
    void ShiftX(double dX);
    void ShiftY(double dY);
    void Normalization(double Xc, double Yc, double Rad, double ScaleFactor = 1.);
    void Clear();
  };
#endif
