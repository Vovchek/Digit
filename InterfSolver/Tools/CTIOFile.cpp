#include "CTIOFile.h"
#include <charconv>
//=========================================================================
CTextIOFile::CTextIOFile()
{
	EndOfSection = "[";
	ItemBreak = ';';
	isInSection = FALSE;
	isBreakControl = TRUE;
}
//=========================================================================
CTextIOFile::CTextIOFile(LPCTSTR FileName, UINT OpenMode) : CStdioFile(FileName, OpenMode)
{
	EndOfSection = "[";
	ItemBreak = ';';
	isInSection = FALSE;
	isBreakControl = TRUE;
}
//=========================================================================
CTextIOFile :: ~CTextIOFile()
{
}
//=========================================================================
void CTextIOFile::SetEndOfSection(LPCTSTR SEndSec)
{
	EndOfSection = SEndSec;
}
//=========================================================================
void CTextIOFile::GetEndOfSection(CString& SEndSec)
{
	SEndSec = EndOfSection;
}
//=========================================================================
BOOL CTextIOFile::GetStringAfter(LPCTSTR IdentName, CString& Str)
{
	CString S, Si;
	int i, iPos, Ns, Ni;

	Si = IdentName;
	Ni = Si.GetLength();
	Str.Empty();
	SeekToBegin();
	while (ReadString(S))
	{
		CutStringToBreak(S);
		if (S.IsEmpty())
			continue;
		if ((iPos = S.Find(IdentName)) > -1)
		{
			Ns = S.GetLength();
			i = iPos + Ni;
			while (i < Ns && (S[i] == ' ' || S[i] == '='))i++;
			Str = S.Mid(i);
			return TRUE;
		}
	}
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::GetStringAfter(LPCTSTR SectionName, LPCTSTR IdentName, CString& Str)
{
	CString S, Si;
	int i, iPos, Ns, Ni;

	Si = IdentName;
	Ni = Si.GetLength();
	Str.Empty();

	if (!SeekToSection(SectionName))
		return FALSE;

	while (ReadStringInSection(S))
	{
		if (S.IsEmpty())
			continue;
		if ((iPos = S.Find(IdentName)) > -1)
		{
			Ns = S.GetLength();
			i = iPos + Ni;
			while (i < Ns && (S[i] == ' ' || S[i] == '='))i++;
			Str = S.Mid(i);
			return TRUE;
		}
	}
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::SeekToSection(LPCTSTR SectionName)
{
	CString S;
	int iPos;
	SeekToBegin();
	while (ReadString(S))
	{
		if ((iPos = S.Find(SectionName)) > -1)
		{
			isInSection = TRUE;
			return TRUE;
		}
	}
	isInSection = FALSE;
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::SeekToSection(LPCTSTR SectionName, CString& Str)
{
	CString S;
	int iPos;
	SeekToBegin();
	while (ReadString(S))
	{
		if ((iPos = S.Find(SectionName)) > -1)
		{
			Str = S;
			isInSection = TRUE;
			return TRUE;
		}
	}
	isInSection = FALSE;
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::SeekToString(LPCTSTR IdentName, CString& Str)
{
	CString S;

	Str.Empty();
	SeekToBegin();
	while (ReadString(S))
	{
		CutStringToBreak(S);
		if (S.IsEmpty())
			continue;
		if (S.Find(IdentName) > -1)
		{
			Str = S;
			return TRUE;
		}
	}
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::SeekToString(LPCTSTR SectionName, LPCTSTR IdentName, CString& Str)
{
	CString S;

	Str.Empty();

	if (!SeekToSection(SectionName))
		return FALSE;

	while (ReadStringInSection(S))
	{
		if (S.IsEmpty())
			continue;
		if (S.Find(IdentName) > -1)
		{
			Str = S;
			return TRUE;
		}
	}
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::ReadStringInSection(CString& Str)
{
	CString S;
	int iPos;
	Str.Empty();
	if (!isInSection)
		return FALSE;
	if (!ReadString(S))
	{
		isInSection = FALSE;
		return FALSE;
	}
	CutStringToBreak(S);
	if ((iPos = S.Find(EndOfSection)) > -1)
	{
		if (iPos > 0)
		{
			Str = S.Left(iPos);
			isInSection = FALSE;
			return TRUE;
		}
		else
		{
			isInSection = FALSE;
			return FALSE;
		}
	}
	else
	{
		Str = S;
		return TRUE;
	}
	return FALSE;
}
//=========================================================================
BOOL CTextIOFile::ReadStringWithEnd(LPCTSTR SEndStr, CString& Str)
{
	CString S, SumStr = "";
	int iPos;
	Str.Empty();
	while (ReadStringInSection(S))
	{
		if ((iPos = S.Find(SEndStr)) > -1)
		{
			if (iPos > 0)
				SumStr += ' ' + S.Left(iPos);
			if (!SumStr.IsEmpty())
			{
				Str = SumStr;
				return TRUE;
			}
			else
				return FALSE;
		}
		else
		{
			if (!S.IsEmpty())
				SumStr += ' ' + S;
		}
	}
	if (!SumStr.IsEmpty())
	{
		Str = SumStr;
		return TRUE;
	}
	return FALSE;
}
//=========================================================================
void CTextIOFile::WriteStringWithEnd(LPCTSTR Str, LPCTSTR SEndStr)
{
	CString S;
	S = CString(Str) + SEndStr;
	WriteString(S);
}
//=========================================================================
void CTextIOFile::WriteStringAfter(LPCTSTR IdentName, LPCTSTR Separator, const CString& Str)
{
	CString S;
	S = CString(IdentName) + CString(Separator) + Str;
	WriteStringWithEnd(S);
}
//=========================================================================
void  CTextIOFile::CutStringToBreak(CString& Str)
{
	if (!isBreakControl)
		return;
	int i, Ns;
	Ns = Str.GetLength();
	i = 0;
	while (i < Ns && Str[i] != ItemBreak)i++;
	if (i == 0)
		Str.Empty();
	else
		Str = Str.Left(i);
}
//=========================================================================
CString GetStringFragment(const CString& Str, int NFragm, char CTab, char EStr)
{
	int i, j, iFragm, NStr;
	CString S = "";
	NStr = Str.GetLength();
	if (NStr == 0)
		return S;
	i = j = 0;
	for (iFragm = 0; iFragm < NFragm; iFragm++)
	{
		i = j;
		while (i < NStr && (Str[i] == CTab || Str[i] == '\t') && Str[i] != EStr) i++;
		j = i;
		while (j < NStr && Str[j] != CTab && Str[j] != '\t' && Str[j] != EStr) j++;
	}
	if ((j - i) > 0)
		S = Str.Mid(i, j - i);
	return S;
}
//=========================================================================
void ReplaceInString(CString& Str, char COld, char CNew, int NNew)
{
	int i, j, NS;
	CString SFragm, SNew = "";
	NS = Str.GetLength();
	if (NS == 0)
		return;
	if (NNew == -1)
	{
		Str.Replace(COld, CNew);
		return;
	}
	CString S = Str;
	Str.Empty();
	for (i = 0; i < NNew; i++)
		SNew += CNew;
	i = 0;
	while (!S.IsEmpty())
	{
		NS = S.GetLength();
		i = 0;
		while (i < NS && S[i] == COld) i++;
		j = i;
		while (j < NS && S[j] != COld) j++;
		if ((j - i) > 0)
		{
			SFragm = SNew + S.Mid(i, j - i);
			Str += SFragm;
		}
		S = S.Mid(j);
	}
}
//==============================================================
// TODO: replace with standard formated reading, 
// add validity check and default values assignment
void FormArrFromString(const CString& Str, CArrayDouble& Arr)
{
	int in, ik, NStr;
	double R;
	Arr.RemoveAll();
	NStr = Str.GetLength();
	if (NStr == 0)
		return;
	CString S;
	in = 0;
	while (in < NStr)
	{
		while (in < NStr && (Str[in] == ' ' || Str[in] == '\t')) in++;
		if (in == NStr)
			break;
		ik = in;
		while (ik < NStr && Str[ik] != ' ' && Str[ik] != '\t') ik++;

		S = Str.Mid(in, ik - in);
		const char* start = (const char*)(LPCTSTR)S;
		const char* end = start + S.GetLength();
		auto [ptr, ec] = std::from_chars(start, end, R);
		if (ec == std::errc()) {
			Arr.Add(R);
		}
		in = ik;
	}
}
//==============================================================
void FormArrFromString(const CString& Str, CArrayInt& Arr)
{
	int in, ik, NStr;
	int K;
	Arr.RemoveAll();
	NStr = Str.GetLength();
	if (NStr == 0)
		return;
	CString S;
	in = 0;
	while (in < NStr)
	{
		while (in < NStr && (Str[in] == ' ' || Str[in] == '\t')) in++;
		if (in == NStr)
			break;
		ik = in;
		while (ik < NStr && Str[ik] != ' ' && Str[ik] != '\t') ik++;
		S = Str.Mid(in, ik - in);
		K = atoi(S);
		Arr.Add(K);
		in = ik;
	}
}
//=========================================================================
void CutStringToBreak(CString& Str, char ItemBreak)
{
	const auto Ns = Str.GetLength();
	for (auto i = 0; i < Ns; ++i) {
		if (Str[i] == ItemBreak) {
			Str = Str.Left(i);
			break;
		}
	}
}
//=========================================================================
BOOL DeleteFilesInDirectory(const CString& DirPath, LPCTSTR Mask)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	CString FileName, Shablon, FullFileName;

	Shablon = DirPath + '\\' + Mask;
	hFind = FindFirstFile(Shablon, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	do
	{
		FileName = FindFileData.cFileName;
		if (FileName[0] == '.' || FindFileData.dwFileAttributes == 48/*FILE_ATTRIBUTE_DIRECTORY*/)
			continue;
		FullFileName = DirPath + '\\' + FileName;
		DeleteFile(FullFileName);
	} while (FindNextFile(hFind, &FindFileData));
	return TRUE;
}
//=========================================================================
BOOL FindFilesInDirectory(const CString& DirPath, LPCTSTR Mask, CStringArray& ArrFileNames)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	CString FileName, Shablon, FullFileName;

	ArrFileNames.RemoveAll();

	Shablon = DirPath + '\\' + Mask;
	hFind = FindFirstFile(Shablon, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	do
	{
		FileName = FindFileData.cFileName;
		if (FileName[0] == '.' || FindFileData.dwFileAttributes == 48/*FILE_ATTRIBUTE_DIRECTORY*/)
			continue;
		FullFileName = DirPath + '\\' + FileName;
		ArrFileNames.Add(FullFileName);
	} while (FindNextFile(hFind, &FindFileData));
	return TRUE;
}
//=========================================================================
void CreateDirectory(const CString& DirPath)
{
	CString CDir, TDir;
	int iPos;
	TDir = DirPath + "\\";
	iPos = TDir.Find('\\');
	CDir = TDir.Left(iPos + 1);
	TDir = TDir.Mid(iPos + 1);
	while ((iPos = TDir.Find('\\')) > -1)
	{
		CDir += TDir.Left(iPos + 1);
		if (GetFileAttributes(CDir) == -1)
			CreateDirectory(CDir, NULL);
		TDir = TDir.Mid(iPos + 1);
	}
}
//=========================================================================
/*
BOOL ClearDirectory(const CString &DirPath, CStringArray &ArrDir)
  {
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  CString FileName, Shablon, SubDirPath, FullFileName;

  Shablon = DirPath + '\\' + "*.*";
  hFind = FindFirstFile(Shablon, &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE)
	return FALSE;
  while(FindNextFile(hFind, &FindFileData))
	{
	FileName = FindFileData.cFileName;
	if (FileName[0] == '.' && FileName.GetLength() < 3)
	  continue;
	if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	  {
	  SubDirPath = DirPath + '\\' + FileName;
	  ArrDir.Add(SubDirPath);
	  ClearDirectory(SubDirPath, ArrDir);
	  continue;
	  }
	FullFileName = DirPath + '\\' + FileName;
	DeleteFile(FullFileName);
	}
  return TRUE;
  }
*/
//=========================================================================
