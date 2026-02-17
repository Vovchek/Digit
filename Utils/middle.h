#if !defined(_MIDDLESTRIP_H_)
#define _MIDDLESTRIP_H_
//C:\Ilya\Programming\cpp\Numbering\Utils\middle.h
#include "stdafx.h"
#include <math.h>
#include <vector>
#include <functional>
#include "MGTools\Include\Utils\BaseDataType.h"

// refactored version of middle() & approx with simplified interface and improved robustness, 
// intended for use in RedCenterDetector
std::vector<double> middle_(const std::uint8_t* line, std::size_t nx, int y,
	std::function<bool(int x, int y)> IsVisible);
double approx_(int* n, int* x, int* y);
void fon_del_(uint8_t* line, int leftIdx, int rightIdx);

// legacy/deprecated versions of middle() and approx() with original interface, still used by some legacy code
void middle(unsigned char* line, int nx, int ny, int y, int **buf_line, 
			CArray<double, double>& CenterFrg, int& nnpolos);
double approx(int *n,int *x,int *y);

// deprecated adapter legacy to std::sort, not intended for new use
void SortDouble(CArray<double, double>& CenterFrg);

// Additional utility functions for line processing, used by middle() and related code
void fon_del(uint8_t* line, int x , int x1);
void invert_line(uint8_t* line, int x , int x1);
void delet_u(uint8_t* line, int end1, int end2, double aa, double bb);

#endif
