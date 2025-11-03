#if !defined(AFX_MEASURE_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_MEASURE_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_

//#include "D:\Numbering\InterfSolver\StdAfx.h"
//#include "D:\Numbering\Appdef.h"
//#include "D:\Numbering\MGTools\Include\Utils\BaseDataType.h"
//C:\Ilya\Programming\cpp\Numbering\Controls\MeasureCtrls.h
#include "InterfSolver\StdAfx.h"
#include "Appdef.h"
#include "MGTools\Include\Utils\BaseDataType.h"


class CMeasureCtrls
{
public:	
  CDLine L;

public:	
    CMeasureCtrls();
    ~CMeasureCtrls();
	void Init();
    void PrepareNewImage();
    void SetFirstPoint(CRect regWnd, CRect regMatr, CPoint wP);
    void SetSecondPoint(CRect regWnd, CRect regMatr, CPoint wP);

    CMeasureCtrls(const CMeasureCtrls& rhs){
      { operator=(rhs);}
     }

    CMeasureCtrls& operator=(const CMeasureCtrls& rhs);

};

#endif // !defined(AFX_IMAGE_CONTROLS_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
