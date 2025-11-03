#include "stdafx.h"
#include "StateListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLOR_ITEM_HEIGHT 25

/////////////////////////////////////////////////////////////////////////////
// CStateListBox

CStateListBox::CStateListBox()
{
}

CStateListBox::~CStateListBox()
{
}


BEGIN_MESSAGE_MAP(CStateListBox, CCheckListBox)
	//{{AFX_MSG_MAP(CStateListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CStateListBox::SetListItems(CStringArray& Str, CUIntArray& State)
{
	for(int i=0; i < Str.GetSize(); i++){
      AddString(LPCTSTR(Str[i]));
      int iS = GetCount()-1;
      if(State[i] == 1)
	    SetCheck(iS, 1);
	}
}

void CStateListBox::GetStateList(CUIntArray& State)
{
	for(int i=0; i < GetCount(); i++){
	  State[i] = GetCheck(i);	
	}
}

/////////////////////////////////////////////////////////////////////////////
// CStateListBox message handlers

// Переопределяем функцию для сравнения строк списка
int CStateListBox::CompareItem(
  LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
/*	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2

  COLORREF cr1 = (COLORREF)lpCompareItemStruct->itemData1;
  COLORREF cr2 = (COLORREF)lpCompareItemStruct->itemData2;

  if (cr1 == cr2)
   return 0; // строки одинаковы

  // Сначала сортирем строки по интенсивности
  int intensity1 = GetRValue(cr1) + 
                   GetGValue(cr1) +
                   GetBValue(cr1);

  int intensity2 = GetRValue(cr2) + 
                   GetGValue(cr2) + 
                   GetBValue(cr2);

  if (intensity1 < intensity2)
    return -1; // сначала идут строки с меньшей яркостью
  else if (intensity1 > intensity2)
    return 1; // более яркие строки идут вторыми

  // Если строки имеют одинаковую интенсивность, 
  // то сортируем по цветам:
  //   первый - синий, последний - красный
  if (GetBValue(cr1) > GetBValue(cr2))
    return -1;
  else if (GetGValue(cr1) > GetGValue(cr2))
    return -1;
  else if (GetRValue(cr1) > GetRValue(cr2))
    return -1;
  else
    return 1;
*/
  return 0;
}

// переопределяем функцию для рисования строк списка
// причем рисуем только содержимое каждого из
// элементов - квадрат и знак в нем (выбрано/не выбрано)
// рисуют без нашего участия 

void CStateListBox::DrawItem(
  LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
  LPCTSTR text = (LPCTSTR)lpDrawItemStruct->itemData;
  CRect rect(lpDrawItemStruct->rcItem);

/*  if (lpDrawItemStruct->itemAction & ODA_DRAWENTIRE)
  {
    CBrush br(RGB(192,192,192));
    pDC->FrameRect(&rect, &br);
  }*/
  // выводим текст элемента управления
  pDC->SetBkMode(TRANSPARENT);
  pDC->DrawText(text, &rect, DT_LEFT | DT_WORDBREAK);
/*
  if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
    (lpDrawItemStruct->itemAction & 
    (ODA_SELECT | ODA_DRAWENTIRE)))
  {
    // Выделение рамки если запись выделена
    COLORREF crHilite = RGB(255-GetRValue(cr),
                            255-GetGValue(cr), 
                            255-GetBValue(cr));

    CBrush br(crHilite);
    pDC->FrameRect(&lpDrawItemStruct->rcItem, &br);
  }

  if (!(lpDrawItemStruct->itemState & ODS_SELECTED) &&
    (lpDrawItemStruct->itemAction & ODA_SELECT))
  {
    // Удаление рамки если выделение с записи списка снято
    CBrush br(cr);
    pDC->FrameRect(&lpDrawItemStruct->rcItem, &br);
  }*/
}

// Переопределяем функцию для определения размеров строк списка
void CStateListBox::MeasureItem(
  LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
  lpMeasureItemStruct->itemHeight = COLOR_ITEM_HEIGHT;
}

