#include "Matrix.h"

//=====================================================================================================================
Matrix :: Matrix()
  {
  Nx= 0;
  Ny= 0;
  }
//============================================================================
Matrix :: Matrix(int My,int Mx)
  {
  Nx=Mx;
  Ny=My;
  Matr.SetSize(Mx * My);
  }
//============================================================================
Matrix :: Matrix(const Matrix &A)
  {
  Nx = A.GetSizeX();
  Ny = A.GetSizeY();
  Matr.Copy(A.Matr);
  }
//============================================================================
Matrix :: ~Matrix()
  {
  Nx= 0;
  Ny= 0;
  Matr.RemoveAll();
  }
//============================================================================
void Matrix :: SetSize(int My, int Mx)
  {
  Nx=Mx;
  Ny=My;
  Matr.RemoveAll();
  Matr.SetSize(Mx * My);
  }
//============================================================================
void Matrix :: Init(int My, int Mx)
  {
  SetSize(My, Mx);
  for (int i = 0; i < My; i++)
    for (int j = 0; j < Mx; j++)
      {
      Matr[i*Nx+j] = 0.;
      if (i == j)
         Matr[i*Nx+j] = 1.;
      }
  }
//============================================================================
void Matrix :: Init(double R, int My, int Mx)
  {
  SetSize(My, Mx);
  for (int i = 0; i < My; i++)
    for (int j = 0; j < Mx; j++)
      {
      Matr[i*Nx+j] = R;
      }
  }
//============================================================================
void Matrix :: InitColumns(Vector A0, ...)
  {
  int i, j;
	va_list ap;
  Vector Ai(Ny);

	va_start(ap, A0);
  for (i = 0; i < Ny; i++) 
    Matr[i*Nx] = A0[i];

  for (j = 1; j < Nx; j++) 
    {
    Ai = va_arg(ap, Vector);
    for (i = 0; i < Ny; i++) 
      Matr[i*Nx+j] = Ai[i];
    }
	va_end(ap);
  }
//============================================================================
void Matrix :: InitRows(Vector A0, ...)
  {
  int i, j;
	va_list ap;
  Vector Ai(Ny);

	va_start(ap, A0);
  for (j = 0; j < Nx; j++) 
    Matr[j] = A0[j];

  for (i = 1; i < Ny; i++) 
    {
    Ai = va_arg(ap, Vector);
    for (j = 0; j < Nx; j++) 
      Matr[i*Nx+j] = Ai[j];
    }
	va_end(ap);
  }
//============================================================================
void Matrix :: Delete(void)
  {
  Nx= 0;
  Ny= 0;
  Matr.RemoveAll();
  }
//============================================================================
void Matrix :: Clear(void)
  {
  Delete();
  }
//============================================================================
Matrix& Matrix :: operator= (const Matrix &A)
  {
  if (this == &A) 
    return *this;
  if (Nx == A.GetSizeX() && Ny == A.GetSizeY()) 
    {
    for (int iy = 0; iy < Ny; iy++) 
      {
      for (int ix = 0; ix < Nx; ix++) 
        Matr[iy*Nx+ix] = A(iy,ix);
      }
    }
  else 
    {
    Nx= A.GetSizeX();
    Ny= A.GetSizeY();
    Matr.SetSize(Nx * Ny);
    for (int iy = 0; iy < Ny; iy++) 
      {
      for (int ix = 0; ix < Nx; ix++) 
        Matr[iy*Nx+ix] = A(iy,ix);
			}
    }
  return *this;
  }
//============================================================================
Matrix Matrix :: Extract(int in, int jn, int Ni, int Nj)
  {
  int i, j;
  Matrix MExt(Nj,Ni);
  for (i = 0; i < Ni; i++) 
    {
    for (j = 0; j < Nj; j++) 
      MExt(i,j) = Matr[(in+i)*Nx+(jn+j)];
    }
  return MExt;
  }
//============================================================================
Matrix operator+ (const Matrix &A, const Matrix &B)
  {
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();
  Matrix C(My,Mx);
  for(int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(iy,ix) = A(iy,ix) + B(iy,ix);
		}
	return C;
  }
//============================================================================
Matrix operator- (const Matrix &A, const Matrix &B)
  {
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();
  Matrix C(My,Mx);
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix=0; ix < Mx; ix++) 
      C(iy,ix) = A(iy,ix) - B(iy,ix);
		}
	return C;
  }
//============================================================================
Matrix operator* (const Matrix &A, const Matrix &B)
{
  int NxC = B.GetSizeX();
  int NyC = A.GetSizeY();
  int NxA = A.GetSizeX();
  Matrix C(NyC,NxC);
  for (int iy = 0; iy < NyC; iy++) 
    {
    for (int ix = 0; ix < NxC; ix++) 
      {
      double S = 0.;
      for (int k = 0; k < NxA; k++) 
        S += A(iy,k) * B(k,ix);
      C(iy,ix) = S;
      }
    }
	return C;
  }
//============================================================================
Matrix operator* (double b, const Matrix &A)
  {
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();
  Matrix C(My,Mx);
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(iy,ix) = b * A(iy,ix);
		}
	return C;
  }
//============================================================================
Matrix operator* (const Matrix &A, double  b)
  {
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();
  Matrix C(My,Mx);
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(iy,ix) = b * A(iy,ix);
		}
	return C;
  }
//============================================================================
Matrix operator/ (const Matrix &A, double  b)
  {
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();
  Matrix C(My,Mx);
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(iy,ix) = A(iy,ix) / b;
		}
	return C;
  }
//============================================================================
Matrix operator^ (const Matrix &A, int NPow)
  {
  Matrix C;
  if (NPow == -1)
    C = Inverse(A);
  else if (NPow == 1)
    C = A;
  else if (NPow == 0)
    {
    int My = A.GetSizeY(); 
    int Mx = A.GetSizeX(); 
    C.Init(My, Mx);
    }
  else if (NPow > 1)
    C = Pow(A, NPow);
  return C;
  }
//============================================================================
Matrix  operator% (const Vector &A, const Vector &B)
  {
  int i, j;
  int Mv=A.GetSize();
  Matrix M(Mv,Mv);
  for (i = 0; i < Mv; i++) 
    for (j = 0; j < Mv; j++) 
      M(i,j)= A[i] * B[j];
  return M;
  }
//============================================================================
Matrix Pow(const Matrix &A,int NPow)
  {
  Matrix MRes;
  int n = A.GetSizeX();
  MRes.Init(n,n);
  CArray<Matrix,Matrix> MulMtr;
  CArrayInt IndPow;
  int RestPow = NPow;
  int iPow;
  while (RestPow > 0)
    {
    int S = 1;
    iPow = 0;
    while (S <= RestPow)
      {
      S *= 2;
      iPow++;
      }
    S /= 2;
    iPow--;
    IndPow.Add(iPow);
    RestPow -= S;
    }
  int NInd = IndPow.GetSize();
  int iInd = NInd - 1;
  int MaxPow = IndPow[0];
  Matrix At(n,n);
  At = A;
  for (iPow = 0; iPow < MaxPow; iPow++ )
    {
    if (iPow == IndPow[iInd])
      {
      MulMtr.Add(At);
      iInd--;
      }
    At = At * At;
    }
  MulMtr.Add(At);

  for (int iMul = 0; iMul < NInd; iMul++)
     MRes = MRes * MulMtr[iMul];
  return MRes;
  }
//============================================================================
double  Sp(const Matrix &A)
  {
  int m = A.GetSizeX();
  double  S = 0.;
  for (int i = 0; i < m; i++) 
    S += A(i,i);
  return S;
  }
//============================================================================
double  Determ(const Matrix &A)
  {
  int M = A.GetSizeX();
  double  Det;
  if (M == 1)
    Det = A(0,0);
  else if (M == 2)
    Det = A(0,0) * A(1,1) - A(1,0) * A(0,1);
  else if (M == 3)
    Det = A(0,0) * A(1,1) * A(2,2) +
          A(0,1) * A(1,2) * A(2,0) + 
          A(0,2) * A(1,0) * A(2,1) -
          A(0,2) * A(1,1) * A(2,0) -
          A(0,0) * A(1,2) * A(2,1) -
          A(0,1) * A(1,0) * A(2,2);
  return Det;
  }
//============================================================================
Matrix Trans(const Matrix &A)
  {
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();
  Matrix C(Mx,My);  // Fixed: transposed dimensions
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(ix,iy) = A(iy,ix);  // Swap indices for transpose
		}
	return C;
  }
//============================================================================
Matrix Inverse(const Matrix &A)
  {
  int NMatr = A.GetSizeY();
  Matrix C;
  Matrix D(NMatr,NMatr);
  C.Init(NMatr,NMatr);
  D = A;
  SystemSolution(D, C, NMatr);
  return C;
  }
//============================================================================
Vector operator* (const Matrix &A,const Vector &b)
  {
  int Mv = b.GetSize();
  int Mx = A.GetSizeX();
  int My = A.GetSizeY();

  Vector C(Mv);
  for (int iy = 0; iy < My; iy++) 
    {
    double  S = 0.;
    for(int ix = 0; ix < Mx; ix++) 
      S += A(iy,ix) * b[ix];
    C[iy] = S;
		}
	return C;
  }
//============================================================================
void Matrix :: Print(LPCTSTR Title)
  {
  FILE *fp;
  fp = fopen("Vecmatr.deb","a");
  fprintf(fp,"\n  %s \n", Title);
  for (int iy = 0; iy < Ny; iy++) 
    {
    for (int ix = 0; ix < Nx; ix++) 
      fprintf(fp, " %11.3le",Matr[iy * Nx + ix]);
    fprintf(fp,"\n");
    }
  fprintf(fp,"\n");
  fclose(fp);
  }
//============================================================================
void Matrix :: Print(LPCTSTR Title, CString &Str)
  {
  CString S;
  Str = "";
  S.Format("\r\n  %s \r\n", Title);
  Str += S;
  for (int iy = 0; iy < Ny; iy++) 
    {
    for (int ix = 0; ix < Nx; ix++)
      { 
      S.Format(" %11.3le", Matr[iy * Nx + ix]);
      Str += S;
      }
    S.Format("\n");
    Str += S;
    }
  S.Format("\n");
  Str += S;
  }
//============================================================================
bool SystemSolution(Matrix &Matr, Vector &B)
  {
  int NMatr = Matr.GetSizeY();
  Matrix Mb(NMatr,1);
  int i;

  for (i = 0; i < NMatr; i++)
    Mb[i] = B[i];

  bool isSuccess;
  isSuccess = SystemSolution(Matr, Mb, 1);

  for (i = 0; i < NMatr; i++)
    B[i] = Mb[i];

  return isSuccess;
  }
//============================================================================
bool SystemSolution (Matrix &Matr, Matrix &B, int NB)
  {
  int NMatr = Matr.GetSizeY();
  bool isSuccess;
  isSuccess = SystemSolution(Matr, NMatr, B, NB);
  return isSuccess;
  }
//============================================================================
bool SystemSolution(Matrix &Matr, int NMatr, Matrix &B, int NB)
  {
  int i, j, k, iMax, jB, l;
  double Guide, Save, Mul, Sum;
  for (k = 0; k < NMatr; k++)
    {
//  Поиск ведущего элемента в столбце
    Guide = Matr(k,k);
	iMax = k;
    for (i = k+1; i < NMatr; i++)
      {
      if (fabs(Matr(i,k)) > fabs(Guide))
        {
        Guide = Matr(i,k);
        iMax = i;
        }
      }
//  Проверка на значимость
    if (fabs(Guide) == 0.)
      return false;
//  Перестановка строк
    for (j = k; j < NMatr; j++)
      {
      Save = Matr(iMax,j);
      Matr(iMax,j) = Matr(k,j);
      Matr(k,j) = Save / Guide;
      }
    for (j = 0; j < NB; j++)
      {
      Save = B(iMax,j);
      B(iMax,j) = B(k,j);
      B(k,j) = Save / Guide;
      }
// Вычитание строк из последующих
    for (i = k+1; i < NMatr; i++)
      {
      Mul = Matr(i,k);
      for (j = k; j < NMatr; j++)
        Matr(i,j) = Matr(i,j) - Matr(k,j) * Mul;
      for (j = 0; j < NB; j++)
        B(i,j) = B(i,j) - B(k,j) * Mul;
      }
    }
// Обратная процедура - раскрытие системы
  for (k = NMatr-1; k >=0; k--)
    {
    for (jB = 0; jB < NB; jB++)
      {
      Sum = 0.;
      for (l = k+1; l < NMatr; l++)
        Sum += Matr(k,l) * B(l,jB);
      B(k,jB)= B(k,jB)- Sum;
      } 
    }
  return true;
  }
//============================================================================
