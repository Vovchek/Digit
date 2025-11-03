#if !defined(_MIDDLESTRIP_H_)
#define _MIDDLESTRIP_H_
//C:\Ilya\Programming\cpp\Numbering\Utils\middle.h
#include "stdafx.h"
#include <math.h>
#include "MGTools\Include\Utils\BaseDataType.h"

void middle(unsigned char* line, int nx, int ny, int y, int **buf_line, 
			CArray<double, double>& CenterFrg, int& nnpolos);
double approx(int *n,int *x,int *y);
void SortDouble(CArray<double, double>& CenterFrg);
void fon_del(unsigned char* line, int x , int x1);
void invert_line(unsigned char* line, int x , int x1);
void delet_u(unsigned char* line, int end1, int end2, double aa, double bb);

#endif
