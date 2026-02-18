#ifndef __NUMBERINFO_H
#define __NUMBERINFO_H

#include "InterfSolver\include\CArrDef.h"
#include "InterfSolver\include\Int_Cons.h"
#include "XYEllipse.h"
#include "XYPolygon.h"
#include "XYRect.h"
#include "XYBounds.h"
#include "SamplDat.h"

struct NUMBERING_INTERFEROGRAM_INFO
{
	// The type of a file from which the interferogram was loaded
	// Primarily used to handle missing image size in dos zap files
	// needed to transform Y-coordinate direction on file load
	enum { TYP_IMAGE = 0, TYP_ZAP_DOS, TYP_ZAP_WIN, TYP_FRN } LoadedFileType{ TYP_IMAGE };
	//
	CString Title;
	CString Date;
	CString Time;
	double ScaleFactor;
	double FiScan;
	CArrayXYEllipse ArrEll;
	CArrayXYRect ArrRect;
	CArrayXYPolygon ArrPlg;
	XYBounds EBnd;
	SAMPLE_DATA DigitDat;
	int ImageSize[2];
	CString ImageFileName;

	NUMBERING_INTERFEROGRAM_INFO();
	NUMBERING_INTERFEROGRAM_INFO(const NUMBERING_INTERFEROGRAM_INFO& A);
	~NUMBERING_INTERFEROGRAM_INFO();
	NUMBERING_INTERFEROGRAM_INFO& operator= (const NUMBERING_INTERFEROGRAM_INFO& A);
	void Clear();
	void GetFringeNumbers(CArrayDouble& ArrFNumbers);
};
#endif
