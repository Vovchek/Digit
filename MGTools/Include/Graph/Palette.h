#ifndef _PALETTE_H_
#define _PALETTE_H_

#include "stdafx.h"
#include <math.h>
#include "MGToolsExport.h"

class MGTOOLS_API CColorPalette   
  {
  public:
  protected:
    RGBQUAD PaletteTab[256];  // Палитра псевдоцветов

  public:
    CColorPalette();
    ~CColorPalette();
    COLORREF GetColor(BYTE Color);
  protected:
    void CreatePalette();
  };
#endif
