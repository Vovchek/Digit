#ifndef __UTIL_H
#define __UTIL_H

#include <afxwin.h>
#include <io.h>
#include <float.h>

#include <stdio.h>
#include <math.h>
#include "fixmem.h"
#include "MGToolsExport.h"

BOOL ReadWindowPlacement(LPWINDOWPLACEMENT pwp);
void WriteWindowPlacement(LPWINDOWPLACEMENT pwp);

BOOL MGTOOLS_API ReadWndPlacement(LPWINDOWPLACEMENT pwp,
                      LPCTSTR Section, LPCTSTR Key, LPCTSTR File);
BOOL MGTOOLS_API WriteWndPlacement(LPWINDOWPLACEMENT pwp,
                       LPCTSTR Section, LPCTSTR Key,
                       LPCTSTR File);

template <class Temp>
void WriteStatic(CWnd *pStatic,Temp Val, char *Format="%d")
{
	char buf[256];
	sprintf(buf,Format,Val);
	pStatic->SetWindowText(buf);
	pStatic->UpdateWindow();
};

double ReadStatic(CWnd *Parent, int Id);

template <class Temp>
void MinMax(Temp *M, int N, Temp &Min, Temp &Max)
{
	Min=Max=M[0];
	for(int i=1; i<N; i++) {
		if(M[i]<Min) Min=M[i];
		if(M[i]>Max) Max=M[i];
	}
}

template <class Temp>
void MinMax(Mem<Temp> &M, Temp &Min, Temp &Max)
{
	Min=Max=M(0);
	int N=M.GetSize();
	for(int i=1; i<N; i++) {
		if(M(i)<Min) Min=M(i);
		if(M(i)>Max) Max=M(i);
	}
}

void EnableControl(CWnd *pDialog, int IdBtn, BOOL State);

BOOL DeleteFile(CString FileName);

double GetWindowsVersion();

template <class T>
int StringToArray(char *Str, T *Array)
{
    char *str=Str;
    char *endptr=Str;

    int N=0;
    int Neg;
    while (*endptr!=0) {
        while(*str==' ') str++;
        if(*str==0) break;
        if(*str=='-') {
            Neg=-1;
            str++;
            while(*str==' ') str++;
        }
        else Neg=1;

        Array[N]=T(Neg*strtod(str, &endptr));
        N++;

        while(*endptr==' ') endptr++;
        str=endptr;

        if(*endptr<'-' || *endptr>'9' || *endptr=='/') str++;
    }
    return N;
}
/*
template <class T>
int StringToArray(CString Str, Mem<T> &A)
{
	if(Str.IsEmpty()) {
		A.SetZero(1);
		return 1;
	}
	A.SetZero(5);
	char *str=Str.GetBuffer(255);
	char *endptr=str;

	int N=0;
	int Neg;
	while (*endptr!=0) {
		while(*str==' ') str++;
		if(*str==0) break;
		if(*str=='-') {
			Neg=-1;
			str++;
			while(*str==' ') str++;
		}
		else Neg=1;

		if(N>=A.GetSize()) A.ReSet(N+5);
		A(N)=Neg*strtod(str, &endptr);
		N++;

		while(*endptr==' ') endptr++;
		str=endptr;

		if(*endptr<'-' || *endptr>'9' || *endptr=='/') str++;
	}
	if(N==0) N=1;
	A.ReSet(N);
	return N;
}
*/
void GetStandardValue(double Value, double &MinValue, double &MaxValue);

void GetMax(int n, double* pArr,
			int& iMax, double& yMax);
void GetMax(int n, double* pArr, int i1, int i2,
			int& iMax, double& yMax);
void GetMin(int n, double* pArr,
			int& iMin, double& yMin);
void GetMin(int n, double* pArr, int i1, int i2,
			int& iMin, double& yMin);
void _MinMax(double *M, int N, double &Min, double &Max);

CString GetAppName(void);                            //e:\work\main.exe
CString GetAppPath(void);                            //e:\work
CString GetFilePath(CString FullFileName);           //e:\work
CString GetFileName(CString FullFileName);           //main.exe
CString GetFileNameWithoutExt(CString FullFileName); //main
CString GetFileExtension(CString FullFileName);      //exe
int GetFilesInDir(CString& Dir, CString Filter, MemString& Files);

void SetCursorArrow(void);
void SetCursorWait(void);

int GetMDIChildNumber(CMDIFrameWnd* pWnd); // возвращает кол. дочерних MDI окон
CString GetRTClassName(HWND hWnd);

void RestoreObjects(CDC& dc);

//чтение из ini-файла
int    GetInt(CString FileName, CString SectionName, CString ParamName);
double GetDouble(CString FileName, CString SectionName, CString ParamName);

BOOL MGTOOLS_API IsFileExist(LPCTSTR fName, BOOL needMessage/*true*/);

//-------------------- File Path ----------------------------------
BOOL MGTOOLS_API SavePath(LPCTSTR Key, LPCTSTR Str, LPCTSTR File);
BOOL MGTOOLS_API ReadPath(LPCTSTR Key, LPTSTR Str, LPCTSTR File);

//-------------------- File Section ----------------------------------
BOOL MGTOOLS_API SaveSection(LPCTSTR Section, LPCTSTR Key, LPCTSTR Str, LPCTSTR File);
BOOL MGTOOLS_API ReadSection(LPCTSTR Section, LPCTSTR Key, LPTSTR Str, LPCTSTR File);

void MGTOOLS_API GetAvaibleDrives(int& nDr, LPTSTR drives);

template <class T>
inline void Swap(T& a, T& b){T t= a; a = b; b = t;}

void MGTOOLS_API CheckLastError();
void MGTOOLS_API MakeLoopMessage();

BOOL MGTOOLS_API CreateTMPUniqueFile(CString& name, LPCTSTR prefix);
BOOL MGTOOLS_API WriteStringToFile(LPCTSTR str, LPCTSTR filepath, UINT nOpenFlags=CFile::modeCreate|CFile::modeWrite);

void CreateEllipseArray(CRect* pR, CPoint* aP, int naP);
void InflateContour(CPoint* pwLine, int nP, int val);

#endif
