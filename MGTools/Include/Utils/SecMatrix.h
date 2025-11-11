#if !defined(AFX_SEC_MATRIX_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_)
#define AFX_SEC_MATRIX_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_

#include "MGToolsExport.h"

class MGTOOLS_API CMatrixSection
{
 public:
    CMatrixSection();
    ~CMatrixSection();

    void SetMatrix(double* _pMatr, int _nx, int _ny);
    void SetMatrix(float* _pMatr, int _nx, int _ny);
    void SetLimits(double _minX, double _maxX, double _minY, double _maxY);
    bool GetSymmetricSection(double Angle, int nStepSec, double* pR, double* pF);
    bool GetLogSymmetricSection(double Angle, int nStepSec, double* pR, double* pF);

 protected:
	 double grd_rd;
	 double rd_grd;
	 double* pMatr;
	 int Nx;
	 int Ny;
	 double minX;
	 double maxX;
	 double minY;
	 double maxY;
	 double dX;
	 double dY;

 protected:
   bool CanCalc();
   double Parinv4P18(double X, double Y);

};

#endif // !defined(AFX_SEC_MATRIX_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_)

