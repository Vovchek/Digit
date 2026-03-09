#ifndef _READWRITEDATA_H_
#define _READWRITEDATA_H_

#include "NunberInfo.h"
#include "CTIOFile.h"

#include "InterfSolver\InterfSolver.h"

INTERF_API BOOL ReadZAPData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API BOOL ReadWinZAPData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API BOOL ReadDosZAPData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API BOOL ReadFRNData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API void WriteWinZAPData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API void WriteDosZAPData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API void WriteFRNData(const CString &FileName, NUMBERING_INTERFEROGRAM_INFO &IntInfo);
INTERF_API void WritePolygon(CTextIOFile &Fl, XYPolygon &Plg, int NPntInStr = 4);
INTERF_API void WriteSection(CTextIOFile &Fl, SAMPLE_DATA &Sampl, int NPntInStr = 6);
INTERF_API void WriteFringe(CTextIOFile &Fl, SAMPLE_DATA &Sampl);
INTERF_API bool CalcWinFringeBoundCircle(const CArray<XYEllipse>& ArrEll, const CArray<XYRect>& ArrRect, double& Xc, double& Yc, double& Rad);
#endif
