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
#define CArrayComplex  CArray<complex>

#define CArrayXYPoint  CArray<XYPoint>
#define CArrayXYBrokenLine  CArray<XYBrokenLine>
#define CArrayXYPolygon  CArray<XYPolygon>
#define CArrayXYEllipse  CArray<XYEllipse>
#define CArrayXYRect  CArray<XYRect>
#define CArrayXYBounds  CArray<XYBounds>

#endif
