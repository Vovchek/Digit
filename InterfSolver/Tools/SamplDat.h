#ifndef __SAMPLDAT_H
#define __SAMPLDAT_H
#include "InterfSolver\include\CArrDef.h"
#include <vector>

// There may be multiple fringes with he same number (discontinuous segments)
// For each dot, we store the Index of the fringe it belongs to
// Fringe's Number has priority over Index
struct FRINGE_ID
{
	// TODO: write clear specification how Index is assigned and used 
    int Index;     // The Index of the fringe segment
    double Number; // The fringe number
};
struct DOT_DATA
{
    double X;      // The X coordinate of the dot
    double Y;      // The Y coordinate of the dot
    double P;      // The property value at the dot
    FRINGE_ID F;   // The fringe's number and segment Index 
                   // the dot belongs to
};

struct SAMPLE_DATA
  {
  public:
    std::vector<DOT_DATA> Dots;
  public:
    SAMPLE_DATA ();
    SAMPLE_DATA& operator= (const SAMPLE_DATA &A);
	DOT_DATA& operator[] (int i) { return Dots[i]; }
    int GetSize();
    void SetSize(int NewSize);
    void Append(const SAMPLE_DATA &A);
    void Add(double X, double Y, double F, double P = 0., int fringe_index = -1);
    void SortIncreaseX();
    void SortDecreaseX();
    void SortIncreaseY();
    void SortDecreaseY();
    void SortIncreaseF();
    void SortDecreaseF();
    void SortIncreaseFY();
    void InverseY(double YcInv);
    void ShiftX(double dX);
    void ShiftY(double dY);
    void Normalization(double Xc, double Yc, double Rad, double ScaleFactor = 1.);
    void Clear();
  };
#endif
