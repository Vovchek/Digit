#include "MeasureCtrls.h"

#include "Utils\mutils.h"

CMeasureCtrls::CMeasureCtrls()
{
	Init();
}

CMeasureCtrls::~CMeasureCtrls()
{
}

void CMeasureCtrls::Init()
{
	PrepareNewImage();
}

void CMeasureCtrls::PrepareNewImage()
{
	L.Init();
}

CMeasureCtrls& CMeasureCtrls::operator=(const CMeasureCtrls& rhs)
{
  if(this==&rhs) return *this;
	 L = rhs.L;
    return *this;
}

void CMeasureCtrls::SetFirstPoint(CRect regWnd, CRect regMatr, CPoint wP)
{
	L.Init();
	CDPoint dP;
	dP.x = wP.x;
	dP.y = wP.y;
	L.P1 = dP;
}

void CMeasureCtrls::SetSecondPoint(CRect regWnd, CRect regMatr, CPoint wP)
{
	if(L.P1.x == 0. && L.P1.y == 0.)
		return;

	CDPoint dP;
	dP.x = wP.x;
	dP.y = wP.y;
	L.P2 = dP;
}

