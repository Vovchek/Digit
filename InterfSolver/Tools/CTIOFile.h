#ifndef __CTIOFILE_H
#define __CTIOFILE_H
#include "stdafx.h"
#include "Include\CArrDef.h"
// OpenMode:
// CFile::modeRead  - only read;
// CFile::modeCreate | CFile::modeWrite  - only write;
// CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate    - Add write;
// Open(LPCTSTR FileName, UINT OpenMode)
// Close()
// See CStdioFile, CFile for more information

class CTextIOFile : public CStdioFile
  {
  protected:
    CString EndOfSection;
    char ItemBreak;
    BOOL isInSection;
    BOOL isBreakControl;
  public:
    CTextIOFile();
    ~CTextIOFile();
    CTextIOFile(LPCTSTR FileName, UINT OpenMode);
    void SetEndOfSection(LPCTSTR SEndSec = "[");
    void GetEndOfSection(CString &SEndSec);
    void SetItemBreak(char _ItemBreak = ';') {isBreakControl = TRUE; ItemBreak = _ItemBreak;}
    char GetItemBreak() {return ItemBreak;}
    void CancelBreakControl() {isBreakControl = FALSE;}
    BOOL GetStringAfter(LPCTSTR IdentName, CString &Str);
    BOOL GetStringAfter(LPCTSTR SectionName, LPCTSTR IdentName, CString &Str);
    BOOL SeekToSection(LPCTSTR SectionName);
    BOOL SeekToSection(LPCTSTR SectionName, CString &Str);
    BOOL SeekToString(LPCTSTR IdentName, CString &Str);
    BOOL SeekToString(LPCTSTR SectionName, LPCTSTR IdentName, CString &Str);
    BOOL ReadStringInSection(CString &Str);
    BOOL ReadStringWithEnd(LPCTSTR SEndStr, CString &Str);
    void WriteStringWithEnd(LPCTSTR Str, LPCTSTR SEndStr = "\n");
    void WriteStringAfter(LPCTSTR IdentName, LPCTSTR Separator, const CString &Str);
  protected:
    void CutStringToBreak(CString &Str);
  };

CString GetStringFragment(const CString &Str, int NFragm, char CTab=' ', char EStr=';');
void ReplaceInString(CString &Str, char COld, char CNew, int NNew);
void CutStringToBreak(CString &Str, char ItemBreak);
void FormArrFromString (const CString &Str, CArrayDouble &Arr);
void FormArrFromString (const CString &Str, CArrayInt &Arr);
BOOL DeleteFilesInDirectory(const CString &DirPath, LPCTSTR Mask);
BOOL FindFilesInDirectory(const CString &DirPath, LPCTSTR Mask, CStringArray &ArrFileNames);
void CreateDirectory(const CString &DirPath);
BOOL ClearDirectory(const CString &DirPath);
#endif
