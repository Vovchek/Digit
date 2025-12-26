#include <math.h>
#include "Include\Int_Cons.h"
#include "GetTimeDate.h"
#include "CalcLimits.h"
#include "ReadWriteData.h"


static bool HasValidFIDS(CTextIOFile& Fl)
{
	CString Str;
	Fl.GetStringAfter("FIDS", Str);
	if (Fl.GetStringAfter("FIDS", Str)) {
		CutStringToBreak(Str, 'E');
		CArrayDouble Buf;
		FormArrFromString(Str, Buf);
		return (Buf.GetSize() == 8);
	}

	return false;
}

//=========================================================================
BOOL ReadZAPData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	CTextIOFile Fl;
	if (!Fl.Open(FileName, CFile::modeRead))
	{
		CString Mes = "Fail to load " + FileName;
		AfxMessageBox(LPCTSTR(Mes), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	auto KeyDos = HasValidFIDS(Fl);

	Fl.Close(); // though RAII, close to avoid shared access

	if (!KeyDos) {
		IntInfo.LoadedFileType = NUMBERING_INTERFEROGRAM_INFO::TYP_ZAP_WIN;
		return ReadWinZAPData(FileName, IntInfo);
	}
	else {
		IntInfo.LoadedFileType = NUMBERING_INTERFEROGRAM_INFO::TYP_ZAP_DOS;
		return ReadDosZAPData(FileName, IntInfo);
	}
}
//=========================================================================
BOOL ReadWinZAPData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	CString Str;
	CTextIOFile Fl;

	IntInfo.Clear();

	if (!Fl.Open(FileName, CFile::modeRead))
	{
		CString Mes = "Fail to load " + FileName;
		AfxMessageBox(LPCTSTR(Mes), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	CArrayDouble Buf;

	if (Fl.GetStringAfter("[GENERAL]", "Title", Str))
		IntInfo.Title = Str;
	if (Fl.GetStringAfter("[GENERAL]", "Date", Str))
		IntInfo.Date = Str;
	if (Fl.GetStringAfter("[GENERAL]", "Time", Str))
		IntInfo.Time = Str;
	if (Fl.GetStringAfter("[GENERAL]", "ScaleFactor", Str))
		IntInfo.ScaleFactor = atof(Str);
	if (Fl.GetStringAfter("[GENERAL]", "FiScan", Str))
		IntInfo.FiScan = atof(Str);
	if (Fl.GetStringAfter("[General]", "IMAGE", Str))
		IntInfo.ImageFileName = Str;


	if (Fl.SeekToSection("[ELLIPSES]"))
	{
		XYEllipse Ell0;
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			Ell0 = XYEllipse(Buf[0], Buf[1], Buf[2], Buf[3], Buf[4], int(Buf[5]), int(Buf[6]));
			IntInfo.ArrEll.Add(Ell0);
		}
	}

	if (Fl.SeekToSection("[RECTANGLES]"))
	{
		XYRect Rect0;
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			Rect0 = XYRect(Buf[0], Buf[1], Buf[2], Buf[3], Buf[4], int(Buf[5]), int(Buf[6]));
			IntInfo.ArrRect.Add(Rect0);
		}
	}

	if (Fl.SeekToSection("[POLYGONS]"))
	{
		XYPolygon Plg0;
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			int TypLim = int(Buf[0]);
			int TypeSystCoor = int(Buf[1]);
			Buf.RemoveAt(0, 2);
			Plg0 = XYPolygon(Buf, TypLim, TypeSystCoor);
			IntInfo.ArrPlg.Add(Plg0);
		}
	}

	if (Fl.SeekToSection("[BOUNDS]"))
	{
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			IntInfo.EBnd = XYBounds(Buf[0], Buf[1], Buf[2], Buf[3]);
			if (IntInfo.EBnd.isEmpty())
				CalcBounds(IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg, IntInfo.EBnd);
		}
	}

	if (!Fl.SeekToSection("[CROSS-SECTIONS]"))
		return FALSE;
	SAMPLE_DATA Sampl;
	int NSampl, NBuf;
	while (Fl.ReadStringWithEnd("E", Str))
	{
		FormArrFromString(Str, Buf);
		NBuf = Buf.GetSize();
		NSampl = (NBuf - 1) / 2;
		Sampl.Clear();
		Sampl.SetSize(NSampl);
		for (int i = 0; i < NSampl; i++)
		{
			Sampl.YPnt[i] = Buf[0];
			Sampl.FPnt[i] = Buf[2 * i + 1];
			Sampl.XPnt[i] = Buf[2 * i + 2];
		}
		IntInfo.DigitDat.Append(Sampl);
	}

	if (Fl.GetStringAfter("[IMAGE]", "Size", Str))
	{
		FormArrFromString(Str, Buf);
		IntInfo.ImageSize[0] = int(Buf[0]);
		IntInfo.ImageSize[1] = int(Buf[1]);
	}
	if (Fl.GetStringAfter("[IMAGE]", "FileName", Str))
		IntInfo.ImageFileName = Str;

	Fl.Close();
	return TRUE;
}
//=========================================================================
BOOL ReadDosZAPData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	int i;
	CString Str;
	CTextIOFile Fl;

	IntInfo.Clear();

	if (!Fl.Open(FileName, CFile::modeRead))
	{
		CString Mes = "Fail to load " + FileName;
		AfxMessageBox(LPCTSTR(Mes), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	CArrayDouble Buf;

	if (Fl.GetStringAfter("TITL", Str))
		IntInfo.Title = Str;
	if (Fl.GetStringAfter("DATE", Str))
		IntInfo.Date = Str;
	if (Fl.GetStringAfter("TIME", Str))
		IntInfo.Time = Str;
	if (Fl.GetStringAfter("NUMBER", Str))
		IntInfo.ScaleFactor = atof(Str);
	if (Fl.GetStringAfter("FISCAN", Str))
		IntInfo.FiScan = atof(Str);
	if (Fl.GetStringAfter("IMAGE", Str))
		IntInfo.ImageFileName = Str;

	// TODO: maybe FIDS stands for 4 feducials, denoting ellipse bounds?
	// restore ellipse bounds
	Fl.GetStringAfter("FIDS", Str);
	CutStringToBreak(Str, 'E');
	FormArrFromString(Str, Buf);
	if (Buf.GetSize() != 8) {
		ASSERT(false);
	}
	double YMin = Buf[1];
	double YMax = Buf[3];
	double XMin = Buf[4];
	double XMax = Buf[6];
	auto Bnd = XYBounds(XMin, YMax, XMax, YMin);
	IntInfo.EBnd = Bnd;
	
	double Xc, Yc, Rad;
	CalcBoundCircle(Bnd, Xc, Yc, Rad);
	
	auto BEll = XYEllipse(Rad, Rad, Xc, Yc);
	IntInfo.ArrEll.Add(BEll);

	if (Fl.SeekToSection("ELLIPS"))
	{
		Fl.SetEndOfSection("END");
		while (Fl.ReadStringInSection(Str))
		{
			FormArrFromString(Str, Buf);
			auto Ell = XYEllipse(Buf[2], Buf[3], Buf[0], Buf[1], Buf[4], int(Buf[5]), 1);
			Ell.DeNormalize(Xc, Yc, Rad);
			IntInfo.ArrEll.Add(Ell);
		}
		Fl.SetEndOfSection();
	}

	// Read sections
	SAMPLE_DATA Sampl;
	int NSampl, NBuf;

	Fl.SeekToSection("FIDS");
	Fl.SetEndOfSection("END");
	while (Fl.ReadStringWithEnd("E", Str))
	{
		FormArrFromString(Str, Buf);
		NBuf = Buf.GetSize();
		NSampl = (NBuf - 1) / 2;
		Sampl.Clear();
		Sampl.SetSize(NSampl);
		for (i = 0; i < NSampl; i++)
		{
			Sampl.YPnt[i] = Buf[0];
			Sampl.FPnt[i] = Buf[2 * i + 1];
			Sampl.XPnt[i] = Buf[2 * i + 2];
		}
		IntInfo.DigitDat.Append(Sampl);
	}

	Fl.Close();

	// TODO: app crashes on inconsistent data, need to check it  

	// Fake image size to inverse Y-coordinates
	// in DOS ZAP files Y-axis is inverted
	// but image size is not stored in the file
	int Nx = int(fabs(IntInfo.EBnd.XRight - IntInfo.EBnd.XLeft) + 0.5);
	int Ny = int(fabs(IntInfo.EBnd.YTop - IntInfo.EBnd.YBottom) + 0.5);

	for (i = 0; i < IntInfo.ArrEll.GetSize(); i++)
		IntInfo.ArrEll[i].InverseY(Ny);
	IntInfo.EBnd.InverseY(Ny);
	IntInfo.DigitDat.InverseY(Ny);

	IntInfo.ImageSize[0] = Nx; // not a real value yet
	IntInfo.ImageSize[1] = Ny; // not a real value yet

	// at that point the data is still inconsistent,
	// it needs shifting Y by (RealImageHeight - NY)  

	return TRUE;

}
//=========================================================================
BOOL ReadFRNData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	CString Str;
	CTextIOFile Fl;

	IntInfo.Clear();
	IntInfo.LoadedFileType = NUMBERING_INTERFEROGRAM_INFO::TYP_FRN;

	if (!Fl.Open(FileName, CFile::modeRead))
	{
		CString Mes = "Fail to load " + FileName;
		AfxMessageBox(LPCTSTR(Mes), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	CArrayDouble Buf;

	if (Fl.GetStringAfter("[GENERAL]", "Title", Str))
		IntInfo.Title = Str;
	if (Fl.GetStringAfter("[GENERAL]", "Date", Str))
		IntInfo.Date = Str;
	if (Fl.GetStringAfter("[GENERAL]", "Time", Str))
		IntInfo.Time = Str;
	if (Fl.GetStringAfter("[GENERAL]", "ScaleFactor", Str))
		IntInfo.ScaleFactor = atof(Str);
	if (Fl.GetStringAfter("[GENERAL]", "FiScan", Str))
		IntInfo.FiScan = atof(Str);


	if (Fl.SeekToSection("[ELLIPSES]"))
	{
		XYEllipse Ell0;
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			Ell0 = XYEllipse(Buf[0], Buf[1], Buf[2], Buf[3], Buf[4], int(Buf[5]), int(Buf[6]));
			IntInfo.ArrEll.Add(Ell0);
		}
	}

	if (Fl.SeekToSection("[RECTANGLES]"))
	{
		XYRect Rect0;
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			Rect0 = XYRect(Buf[0], Buf[1], Buf[2], Buf[3], Buf[4], int(Buf[5]), int(Buf[6]));
			IntInfo.ArrRect.Add(Rect0);
		}
	}

	if (Fl.SeekToSection("[POLYGONS]"))
	{
		XYPolygon Plg0;
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			int TypLim = int(Buf[0]);
			int TypeSystCoor = int(Buf[1]);
			Buf.RemoveAt(0, 2);
			Plg0 = XYPolygon(Buf, TypLim, TypeSystCoor);
			IntInfo.ArrPlg.Add(Plg0);
		}
	}

	if (Fl.GetStringAfter("[IMAGE]", "Size", Str) ||	// Digit format
		Fl.GetStringAfter("[IMAGE_FILE]", "Size", Str)) // WinFringe format)
	{
		FormArrFromString(Str, Buf);
		IntInfo.ImageSize[0] = int(Buf[0]);
		IntInfo.ImageSize[1] = int(Buf[1]);
	}
	if (Fl.GetStringAfter("[IMAGE]", "FileName", Str) || // Digit format
		Fl.GetStringAfter("[IMAGE_FILE]", "Name", Str))  // WinFringe format
		IntInfo.ImageFileName = Str;

	// !!! Place this after [IMAGE] - for WinFringe format we need to know image size 
	// to flip the bounds correctly
	if (Fl.SeekToSection("[BOUNDS]"))
	{
		while (Fl.ReadStringWithEnd("E", Str))
		{
			FormArrFromString(Str, Buf);
			if(Buf.GetSize() == 4) // Digit format: Xl, Yt, Xr, Yb 
				IntInfo.EBnd = XYBounds(Buf[0], Buf[1], Buf[2], Buf[3]);
			else if (Buf.GetSize() == 6) { // WinFringe format: Xl, Xr, Yt, Yb, shape{0|1|2}, feature ?
				IntInfo.EBnd = XYBounds(Buf[0], Buf[2], Buf[1], Buf[3]);
				auto ax = (Buf[1] - Buf[0]) / 2.0;
				auto by = (Buf[3] - Buf[2]) / 2.0;
				auto xc = (Buf[1] + Buf[0]) / 2.0;
				auto yc = (Buf[3] + Buf[2]) / 2.0;
				if (int(Buf[4]) == 0) { // circular or elliptic
					auto Ell0 = XYEllipse(ax, by, xc, yc);
					IntInfo.ArrEll.Add(Ell0);
				}
				else if (int(Buf[4]) == 1) { // ???
				}
				else if (int(Buf[4]) == 2) { // rectangular
					auto Rect0 = XYRect(ax, by, xc, yc);
					IntInfo.ArrRect.Add(Rect0);
				}
			}
			if (IntInfo.EBnd.isEmpty())
				CalcBounds(IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg, IntInfo.EBnd);
		}
	}

	if (!Fl.SeekToSection("[FRINGES]"))
		return FALSE;
	SAMPLE_DATA Sampl;
	int NSampl, NBuf;
	double Fr;
	while (Fl.ReadStringInSection(Str))
	{
		if (Str.IsEmpty())
		{
			if (!Fl.ReadStringInSection(Str))
				break;
		}
		Str = GetStringFragment(Str, 2, '=');
		Fr = atof(Str);
		if (!Fl.ReadStringWithEnd("E", Str))
			break;
		FormArrFromString(Str, Buf);
		NBuf = Buf.GetSize();
		NSampl = NBuf / 2;
		Sampl.Clear();
		Sampl.SetSize(NSampl);
		for (int i = 0; i < NSampl; i++)
		{
			Sampl.XPnt[i] = Buf[2 * i];
			Sampl.YPnt[i] = Buf[2 * i + 1];
			Sampl.FPnt[i] = Fr;
		}
		IntInfo.DigitDat.Append(Sampl);
	}

	Fl.Close();
	return TRUE;
}
//=========================================================================
void WriteWinZAPData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	CString Str;
	int i, NPnt;
	XYPoint P;

	CTextIOFile Fl;

	Fl.Open(FileName, CFile::modeCreate | CFile::modeWrite);
	Fl.WriteStringWithEnd("[GENERAL]");
	Str = IntInfo.Title;
	Fl.WriteStringAfter("Title", "=", Str);
	CString Time, Date;
	GetTimeDate(Time, Date);
	Str = Date;
	Fl.WriteStringAfter("Date", "=", Str);
	Str = Time;
	Fl.WriteStringAfter("Time", "=", Str);
	Str.Format("%-1.3lf", IntInfo.ScaleFactor);
	Fl.WriteStringAfter("ScaleFactor", "=", Str);
	Str.Format("%-1.2lf", IntInfo.FiScan);
	Fl.WriteStringAfter("FiScan", "=", Str);
	Fl.WriteStringWithEnd("");

	int NEll = IntInfo.ArrEll.GetSize();
	if (NEll > 0)
	{
		Fl.WriteStringWithEnd("[ELLIPSES]");
		for (i = 0; i < NEll; i++)
		{
			Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf %1.2lf %1d %1d ",
				IntInfo.ArrEll[i].Ax, IntInfo.ArrEll[i].By,
				IntInfo.ArrEll[i].Xc, IntInfo.ArrEll[i].Yc,
				IntInfo.ArrEll[i].Fi, IntInfo.ArrEll[i].TypeLimits,
				IntInfo.ArrEll[i].TypeSystCoor);
			Fl.WriteStringWithEnd(Str, "E\n");
		}
		Fl.WriteStringWithEnd("");
	}

	int NRect = IntInfo.ArrRect.GetSize();
	if (NRect > 0)
	{
		Fl.WriteStringWithEnd("[RECTANGLES]");
		for (i = 0; i < NRect; i++)
		{
			Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf %1.2lf %1d %1d ",
				IntInfo.ArrRect[i].Ax, IntInfo.ArrRect[i].By,
				IntInfo.ArrRect[i].Xc, IntInfo.ArrRect[i].Yc,
				IntInfo.ArrRect[i].Fi, IntInfo.ArrRect[i].TypeLimits,
				IntInfo.ArrRect[i].TypeSystCoor);
			Fl.WriteStringWithEnd(Str, "E\n");
		}
		Fl.WriteStringWithEnd("");
	}

	int NPlg = IntInfo.ArrPlg.GetSize();
	if (NPlg > 0)
	{
		Fl.WriteStringWithEnd("[POLYGONS]");
		for (i = 0; i < NPlg; i++)
		{
			WritePolygon(Fl, IntInfo.ArrPlg[i]);
		}
		Fl.WriteStringWithEnd("");
	}

	Fl.WriteStringWithEnd("[BOUNDS]");
	CalcBounds(IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg, IntInfo.EBnd);
	Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf", IntInfo.EBnd.XLeft, IntInfo.EBnd.YTop,
		IntInfo.EBnd.XRight, IntInfo.EBnd.YBottom);
	Fl.WriteStringWithEnd(Str, " E\n");
	Fl.WriteStringWithEnd("");

	Fl.WriteStringWithEnd("[CROSS-SECTIONS]");
	double EpsY = fabs(IntInfo.EBnd.YTop - IntInfo.EBnd.YBottom) / IntInfo.ImageSize[1];
	double X, Y, F, YCurSec;
	SAMPLE_DATA Sampl;
	CArrayBool isUse;
	bool isFinish;
	int in;

	NPnt = IntInfo.DigitDat.GetSize();

	isUse.SetSize(NPnt);
	for (i = 0; i < NPnt; i++)
		isUse[i] = false;

	IntInfo.DigitDat.SortIncreaseY();

	Sampl.Clear();
	X = IntInfo.DigitDat.XPnt[0];
	Y = IntInfo.DigitDat.YPnt[0];
	F = IntInfo.DigitDat.FPnt[0];
	YCurSec = Y;
	Sampl.Add(X, Y, F);
	isUse[0] = true;
	in = 1;
	isFinish = false;
	while (!isFinish)
	{
		for (i = in; i < NPnt; i++)
		{
			if (isUse[i])
				continue;
			Y = IntInfo.DigitDat.YPnt[i];
			if (fabs(YCurSec - Y) < EpsY)
			{
				X = IntInfo.DigitDat.XPnt[i];
				F = IntInfo.DigitDat.FPnt[i];
				Sampl.Add(X, Y, F);
				isUse[i] = true;
			}
		}
		if (Sampl.GetSize() > 0)
		{
			Sampl.SortIncreaseX();
			WriteSection(Fl, Sampl);
		}
		for (i = 0; i < NPnt; i++)
		{
			if (isUse[i])
				isFinish = true;
			else
			{
				Sampl.Clear();
				X = IntInfo.DigitDat.XPnt[i];
				Y = IntInfo.DigitDat.YPnt[i];
				F = IntInfo.DigitDat.FPnt[i];
				YCurSec = Y;
				Sampl.Add(X, Y, F);
				isUse[i] = true;
				in = i + 1;
				isFinish = false;
				break;
			}
		}
	}
	Fl.WriteStringWithEnd("");

	Fl.WriteStringWithEnd("[IMAGE]");
	Str.Format("%-d %d", IntInfo.ImageSize[0], IntInfo.ImageSize[1]);
	Fl.WriteStringAfter("Size", "=", Str);
	Str = IntInfo.ImageFileName;
	Fl.WriteStringAfter("FileName", "=", Str);
	Fl.Close();
}
//=========================================================================
void WriteDosZAPData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	CString Str;
	int i, NPnt;
	XYPoint P;

	CTextIOFile Fl;

	Fl.Open(FileName, CFile::modeCreate | CFile::modeWrite);
	Str = IntInfo.Title;
	Fl.WriteStringAfter("TITL", " ", Str);
	CString Time, Date;
	GetTimeDate(Time, Date);
	Str = Time;
	Fl.WriteStringAfter("TIME", " ", Str);
	Str = Date;
	Fl.WriteStringAfter("DATE", " ", Str);
	Str.Format("%-1.3lf", IntInfo.ScaleFactor);
	Fl.WriteStringAfter("NUMBER", " ", Str);
	Str.Format("%-1.2lf", IntInfo.FiScan);
	Fl.WriteStringAfter("FISCAN", " ", Str);
	Fl.WriteStringWithEnd("IMAGE " + IntInfo.ImageFileName + "\n");

	double Xc, Yc, Rad;
	CalcBoundCircle(IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg, Xc, Yc, Rad);
	Yc = double(IntInfo.ImageSize[1]) - Yc;

	int NEll = IntInfo.ArrEll.GetSize();
	XYEllipse Ell;
	if (NEll > 0)
	{
		Fl.WriteStringWithEnd("ELLIPS");
		for (i = 0; i < NEll; i++)
		{
			Ell = IntInfo.ArrEll[i];
			Ell.InverseY(IntInfo.ImageSize[1]);
			Ell.Normalize(Xc, Yc, Rad);
			Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf %1.2lf %1d  ",
				Ell.Xc, Ell.Yc, Ell.Ax, Ell.By, Ell.Fi, Ell.TypeLimits);
			Fl.WriteStringWithEnd(Str);
		}
		Fl.WriteStringWithEnd("END");
	}

	int NRect = IntInfo.ArrRect.GetSize();
	if (NRect > 0)
		AfxMessageBox("The RECT is absent in Dos format ZAP file", MB_OK | MB_ICONEXCLAMATION);

	int NPlg = IntInfo.ArrPlg.GetSize();
	if (NPlg > 0)
		AfxMessageBox("The POLYGON is absent in Dos format ZAP file", MB_OK | MB_ICONEXCLAMATION);

	double XMin = Xc - Rad;
	double YMin = Yc - Rad;
	double XMax = Xc + Rad;
	double YMax = Yc + Rad;

	Str.Format("FIDS %1.3lf %1.3lf %1.3lf %1.3lf %1.3lf %1.3lf %1.3lf %1.3lf",
		Xc, YMin, Xc, YMax, XMin, Yc, XMax, Yc);
	Fl.WriteStringWithEnd(Str, " END\n");

	IntInfo.DigitDat.InverseY(IntInfo.ImageSize[1]);
	double EpsY = fabs(IntInfo.EBnd.YTop - IntInfo.EBnd.YBottom) / IntInfo.ImageSize[1];
	double X, Y, F, YCurSec;
	SAMPLE_DATA Sampl;
	CArrayBool isUse;
	bool isFinish;
	int in;

	NPnt = IntInfo.DigitDat.GetSize();

	isUse.SetSize(NPnt);
	for (i = 0; i < NPnt; i++)
		isUse[i] = false;

	IntInfo.DigitDat.SortIncreaseY();

	Sampl.Clear();
	X = IntInfo.DigitDat.XPnt[0];
	Y = IntInfo.DigitDat.YPnt[0];
	F = IntInfo.DigitDat.FPnt[0];
	YCurSec = Y;
	Sampl.Add(X, Y, F);
	isUse[0] = true;
	in = 1;
	isFinish = false;
	while (!isFinish)
	{
		for (i = in; i < NPnt; i++)
		{
			if (isUse[i])
				continue;
			Y = IntInfo.DigitDat.YPnt[i];
			if (fabs(YCurSec - Y) < EpsY)
			{
				X = IntInfo.DigitDat.XPnt[i];
				F = IntInfo.DigitDat.FPnt[i];
				Sampl.Add(X, Y, F);
				isUse[i] = true;
			}
		}
		if (Sampl.GetSize() > 0)
		{
			Sampl.SortIncreaseX();
			WriteSection(Fl, Sampl, 5);
		}
		for (i = 0; i < NPnt; i++)
		{
			if (isUse[i])
				isFinish = true;
			else
			{
				Sampl.Clear();
				X = IntInfo.DigitDat.XPnt[i];
				Y = IntInfo.DigitDat.YPnt[i];
				F = IntInfo.DigitDat.FPnt[i];
				YCurSec = Y;
				Sampl.Add(X, Y, F);
				isUse[i] = true;
				in = i + 1;
				isFinish = false;
				break;
			}
		}
	}
	Fl.WriteStringWithEnd("END\n");
	Fl.Close();
}
//=========================================================================
void WriteFRNData(const CString& FileName, NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	CString Str;
	int i, NPnt;
	XYPoint P;

	CTextIOFile Fl;

	Fl.Open(FileName, CFile::modeCreate | CFile::modeWrite);
	Fl.WriteStringWithEnd("[GENERAL]");
	Str = IntInfo.Title;
	Fl.WriteStringAfter("Title", "=", Str);
	CString Time, Date;
	GetTimeDate(Time, Date);
	Str = Date;
	Fl.WriteStringAfter("Date", "=", Str);
	Str = Time;
	Fl.WriteStringAfter("Time", "=", Str);
	Str.Format("%-1.3lf", IntInfo.ScaleFactor);
	Fl.WriteStringAfter("ScaleFactor", "=", Str);
	Str.Format("%-1.2lf", IntInfo.FiScan);
	Fl.WriteStringAfter("FiScan", "=", Str);
	Fl.WriteStringWithEnd("");

	int NEll = IntInfo.ArrEll.GetSize();
	if (NEll > 0)
	{
		Fl.WriteStringWithEnd("[ELLIPSES]");
		for (i = 0; i < NEll; i++)
		{
			Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf %1.2lf %1d %1d ",
				IntInfo.ArrEll[i].Ax, IntInfo.ArrEll[i].By,
				IntInfo.ArrEll[i].Xc, IntInfo.ArrEll[i].Yc,
				IntInfo.ArrEll[i].Fi, IntInfo.ArrEll[i].TypeLimits,
				IntInfo.ArrEll[i].TypeSystCoor);
			Fl.WriteStringWithEnd(Str, "E\n");
		}
		Fl.WriteStringWithEnd("");
	}

	int NRect = IntInfo.ArrRect.GetSize();
	if (NRect > 0)
	{
		Fl.WriteStringWithEnd("[RECTANGLES]");
		for (i = 0; i < NRect; i++)
		{
			Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf %1.2lf %1d %1d ",
				IntInfo.ArrRect[i].Ax, IntInfo.ArrRect[i].By,
				IntInfo.ArrRect[i].Xc, IntInfo.ArrRect[i].Yc,
				IntInfo.ArrRect[i].Fi, IntInfo.ArrRect[i].TypeLimits,
				IntInfo.ArrRect[i].TypeSystCoor);
			Fl.WriteStringWithEnd(Str, "E\n");
		}
		Fl.WriteStringWithEnd("");
	}

	int NPlg = IntInfo.ArrPlg.GetSize();
	if (NPlg > 0)
	{
		Fl.WriteStringWithEnd("[POLYGONS]");
		for (i = 0; i < NPlg; i++)
		{
			WritePolygon(Fl, IntInfo.ArrPlg[i]);
		}
		Fl.WriteStringWithEnd("");
	}

	Fl.WriteStringWithEnd("[BOUNDS]");
	CalcBounds(IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg, IntInfo.EBnd);
	Str.Format(" %1.3lf %1.3lf %1.3lf %1.3lf", IntInfo.EBnd.XLeft, IntInfo.EBnd.YTop,
		IntInfo.EBnd.XRight, IntInfo.EBnd.YBottom);
	Fl.WriteStringWithEnd(Str, " E\n");
	Fl.WriteStringWithEnd("");

	Fl.WriteStringWithEnd("[FRINGES]");
	double EpsF = PRECISION;
	double X, Y, F, FCur;
	SAMPLE_DATA Sampl;
	XYBrokenLine Bln;
	CArrayBool isUse;
	bool isFinish;
	int in;

	NPnt = IntInfo.DigitDat.GetSize();

	isUse.SetSize(NPnt);
	for (i = 0; i < NPnt; i++)
		isUse[i] = false;

	IntInfo.DigitDat.SortIncreaseF();

	Sampl.Clear();
	X = IntInfo.DigitDat.XPnt[0];
	Y = IntInfo.DigitDat.YPnt[0];
	F = IntInfo.DigitDat.FPnt[0];
	FCur = F;
	Sampl.Add(X, Y, F);
	isUse[0] = true;
	in = 1;
	isFinish = false;
	while (!isFinish)
	{
		for (i = in; i < NPnt; i++)
		{
			if (isUse[i])
				continue;
			F = IntInfo.DigitDat.FPnt[i];
			if (fabs(FCur - F) < EpsF)
			{
				X = IntInfo.DigitDat.XPnt[i];
				Y = IntInfo.DigitDat.YPnt[i];
				Sampl.Add(X, Y, F);
				isUse[i] = true;
			}
		}
		if (Sampl.GetSize() > 0)
		{
			Bln = XYBrokenLine(Sampl.XPnt, Sampl.YPnt);
			Bln.Arrange();
			Bln.GetArrX(Sampl.XPnt);
			Bln.GetArrY(Sampl.YPnt);
			WriteFringe(Fl, Sampl);
		}
		for (i = 0; i < NPnt; i++)
		{
			if (isUse[i])
				isFinish = true;
			else
			{
				Sampl.Clear();
				X = IntInfo.DigitDat.XPnt[i];
				Y = IntInfo.DigitDat.YPnt[i];
				F = IntInfo.DigitDat.FPnt[i];
				FCur = F;
				Sampl.Add(X, Y, F);
				isUse[i] = true;
				in = i + 1;
				isFinish = false;
				break;
			}
		}
	}
	Fl.WriteStringWithEnd("");

	Fl.WriteStringWithEnd("[IMAGE]");
	Str.Format("%-d %d", IntInfo.ImageSize[0], IntInfo.ImageSize[1]);
	Fl.WriteStringAfter("Size", "=", Str);
	Str = IntInfo.ImageFileName;
	Fl.WriteStringAfter("FileName", "=", Str);
	Fl.Close();
}
//=========================================================================
void WritePolygon(CTextIOFile& Fl, XYPolygon& Plg, int NPntInStr)
{
	CString Str, S;
	int  m, iPnt, NPnt;
	bool isBeg;
	XYPoint P;
	NPnt = Plg.GetSize();
	m = 0;
	isBeg = true;
	Str.Empty();
	for (iPnt = 0; iPnt < NPnt; iPnt++)
	{
		P = Plg[iPnt];
		if (isBeg)
		{
			int TypLim = Plg.GetTypeLimits();
			int TypSys = Plg.GetTypeSystCoor();
			S.Format(" %1d %1d %1.3lf %1.3lf", TypLim, TypSys, P.X, P.Y);
			isBeg = false;
			Str += S;
			m++;
		}
		else if (m == 0)
		{
			S.Format("     %1.3lf %1.3lf", P.X, P.Y);
			Str += S;
			m++;
		}
		else if (m == NPntInStr - 1)
		{
			S.Format(" %1.3lf %1.3lf", P.X, P.Y);
			Str += S;
			if (iPnt == NPnt - 1)
				Fl.WriteStringWithEnd(Str, " E\n");
			else
				Fl.WriteStringWithEnd(Str);
			Str.Empty();
			m = 0;
		}
		else if (m < NPntInStr - 1)
		{
			S.Format(" %1.3lf %1.3lf", P.X, P.Y);
			Str += S;
			m++;
		}
	}
	if (m > 0)
		Fl.WriteStringWithEnd(Str, " E\n");
}
//=========================================================================
void WriteSection(CTextIOFile& Fl, SAMPLE_DATA& Sampl, int NPntInStr)
{
	CString Str, S;
	int  m, iPnt, NPnt;
	bool isBeg;
	double YSec;
	XYPoint P;

	YSec = Sampl.YPnt[0];
	NPnt = Sampl.FPnt.GetSize();
	m = 0;
	isBeg = true;
	Str.Empty();
	for (iPnt = 0; iPnt < NPnt; iPnt++)
	{
		P.X = Sampl.FPnt[iPnt];
		P.Y = Sampl.XPnt[iPnt];
		if (isBeg)
		{
			S.Format(" %1.3lf %1.1lf %1.3lf", YSec, P.X, P.Y);
			isBeg = false;
			Str += S;
			m++;
		}
		else if (m == 0)
		{
			S.Format("        %1.1lf %1.3lf", P.X, P.Y);
			Str += S;
			m++;
		}
		else if (m == NPntInStr - 1)
		{
			S.Format(" %1.1lf %1.3lf", P.X, P.Y);
			Str += S;
			if (iPnt == NPnt - 1)
				Fl.WriteStringWithEnd(Str, " E\n");
			else
				Fl.WriteStringWithEnd(Str);
			Str.Empty();
			m = 0;
		}
		else if (m < NPntInStr - 1)
		{
			S.Format(" %1.1lf %1.3lf", P.X, P.Y);
			Str += S;
			m++;
		}
	}
	if (m > 0)
		Fl.WriteStringWithEnd(Str, " E\n");
}
//=========================================================================
void WriteFringe(CTextIOFile& Fl, SAMPLE_DATA& Sampl)
{
	CString Str;
	int  iPnt, NPnt;
	double Fr;
	XYPoint P;

	NPnt = Sampl.FPnt.GetSize();

	Fr = Sampl.FPnt[0];
	Str.Format("%1.1lf", Fr);
	Fl.WriteStringAfter("NFringe", "=", Str);

	for (iPnt = 0; iPnt < NPnt; iPnt++)
	{
		P.X = Sampl.XPnt[iPnt];
		P.Y = Sampl.YPnt[iPnt];
		Str.Format(" %1.3lf %1.3lf", P.X, P.Y);
		Fl.WriteStringWithEnd(Str);
	}
	Fl.WriteStringWithEnd("E");
}
//=========================================================================
