#ifndef __CARRDEF_H
#define __CARRDEF_H

#include "stdafx.h"
#include "complex.h"
#include <afxtempl.h>

#define CArrayByte     CArray<BYTE,BYTE>
#define CArrayBool     CArray<bool,bool>
#define CArrayChar     CArray<char,char>
#define CArrayShort    CArray<short,short>
#define CArrayInt      CArray<int,int>
#define CArrayLong     CArray<long,long>
#define CArrayFloat    CArray<float,float>
#define CArrayDouble   CArray<double,double>
#define CArrayComplex  CArray<complex,complex>

#define CArrayXYPoint  CArray<XYPoint,XYPoint>
#define CArrayXYBrokenLine  CArray<XYBrokenLine,XYBrokenLine>
#define CArrayXYPolygon  CArray<XYPolygon,XYPolygon>
#define CArrayXYEllipse  CArray<XYEllipse,XYEllipse>
#define CArrayXYRect  CArray<XYRect,XYRect>
#define CArrayXYBounds  CArray<XYBounds,XYBounds>

#endif
