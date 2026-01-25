#include "SamplDat.h"
#include <algorithm>

//==============================================================
SAMPLE_DATA::SAMPLE_DATA()
{
}
//==============================================================
SAMPLE_DATA& SAMPLE_DATA :: operator= (const SAMPLE_DATA& A)
{
	Dots = A.Dots;
	return *this;
}
//==============================================================
int SAMPLE_DATA::GetSize()
{
	return Dots.size();
}
//==============================================================
void SAMPLE_DATA::SetSize(int NewSize)
{
	Dots.resize(NewSize);
}
//==============================================================
void SAMPLE_DATA::Append(const SAMPLE_DATA& A)
{
	Dots.insert(Dots.end(), A.Dots.begin(), A.Dots.end());
}
//==============================================================
void SAMPLE_DATA::Add(double X, double Y, double F, double P, int fringe_index)
{
	Dots.push_back({ X, Y, P, {fringe_index, F} });
}
//==============================================================
void SAMPLE_DATA::SortIncreaseX()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		return a.X < b.X;
		});
}
//==============================================================
void SAMPLE_DATA::SortDecreaseX()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		return a.X > b.X;
		});
}
//==============================================================
void SAMPLE_DATA::SortIncreaseY()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		return a.Y < b.Y;
		});
}
//==============================================================
void SAMPLE_DATA::SortDecreaseY()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		return a.Y > b.Y;
		});
}
//==============================================================
// Number always has priority over Index
void SAMPLE_DATA::SortIncreaseF()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		if (a.F.Number != b.F.Number)
			return a.F.Number < b.F.Number;
		return a.F.Index < b.F.Index;
		});
}
//==============================================================
// Priority: F.Number increasing -> Index increasing -> Y increasing
void SAMPLE_DATA::SortIncreaseFY()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		if (a.F.Number != b.F.Number)
			return a.F.Number < b.F.Number;
		if (a.F.Index != b.F.Index)
			return a.F.Index < b.F.Index;
		return a.Y < b.Y;
		});
}
//==============================================================
// Number always has priority over Index
void SAMPLE_DATA::SortDecreaseF()
{
	std::sort(Dots.begin(), Dots.end(), [](const DOT_DATA& a, const DOT_DATA& b) {
		if (a.F.Number != b.F.Number)
			return a.F.Number > b.F.Number;
		return a.F.Index > b.F.Index;
		});
}
//==============================================================
void SAMPLE_DATA::InverseY(double YcInv)
{
	std::for_each(Dots.begin(), Dots.end(), [YcInv](DOT_DATA& dot) {
		dot.Y = YcInv - dot.Y;
		});
}
//==============================================================
void SAMPLE_DATA::ShiftX(double dX)
{
	std::for_each(Dots.begin(), Dots.end(), [dX](DOT_DATA& dot) {
		dot.X += dX;
		});
}
//==============================================================
void SAMPLE_DATA::ShiftY(double dY)
{
	std::for_each(Dots.begin(), Dots.end(), [dY](DOT_DATA& dot) {
		dot.Y += dY;
		});
}
//==============================================================
void SAMPLE_DATA::Normalization(double Xc, double Yc, double Rad, double ScaleFactor)
{
	std::for_each(Dots.begin(), Dots.end(), [Xc, Yc, Rad, ScaleFactor](DOT_DATA& dot) {
		dot.X = (dot.X - Xc) / Rad;
		dot.Y = (dot.Y - Yc) / Rad;
		dot.P /= ScaleFactor;
		});
}
//==============================================================
void SAMPLE_DATA::Clear()
{
	Dots.clear();
}
//==============================================================
