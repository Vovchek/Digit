#ifndef _CALCAPPRIMAGEMATR_H_
#define _CALCAPPRIMAGEMATR_H_

#include "Tools\NunberInfo.h"
#include "Tools\PolApproximation.h"

BOOL CalcPolAprImageMatr(NUMBERING_INTERFEROGRAM_INFO &IntInfo, int MaxPow, double Eps,
                                                                               Matrix &MApr); 
#endif
