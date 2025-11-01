#ifndef __MATRIX_H
#define __MATRIX_H

#include "stdafx.h"
#include "include\CArrDef.h"
#include "Vector.h"

//===============================================================================
class Matrix
  {
	protected:
    int Nx;
    int Ny;
    CArrayDouble Matr;

	public:
		Matrix(void);
    Matrix(int My, int Mx);
    Matrix(const Matrix &M);
		~Matrix();

    void SetSize(int My, int Mx );
    void Init(int My, int Mx);
    void Init(double R, int My, int Mx);
    void InitColumns(Vector A0, ...);
    void InitRows(Vector A0, ...);
    double* GetMem() {return Matr.GetData();}
    const double* GetMem() const {return Matr.GetData();}
    int GetSizeX() const {return Nx;};
    int GetSizeY() const {return Ny;};
    void Delete();
    void Clear();
    double &operator () (int iy, int ix) {return Matr[iy*Nx+ix];}
    const double operator () (int iy, int ix) const {return Matr[iy*Nx+ix];}
    double& operator[] (int i) {return Matr[i];}
    const double operator[] (int i) const {return Matr[i];}
    Matrix& operator= (const Matrix &A);
    Matrix Extract(int in, int jn, int Ni, int Nj);
    void Print(LPCTSTR Title);
    void Print(LPCTSTR Title, CString &Str);

  friend Matrix  operator+ (const Matrix &A, const Matrix &B);
  friend Matrix  operator- (const Matrix &A, const Matrix &B);
  friend Matrix  operator* (const Matrix &A, const Matrix &B);
  friend Matrix  operator* (double b, const Matrix &A);
  friend Matrix  operator* (const Matrix &A, double b);
  friend Vector  operator* (const Matrix &A, const Vector &b);
  friend Matrix  operator/ (const Matrix &A, double b);
  friend Matrix  operator^ (const Matrix &A, int NPow);
  friend Matrix  operator% (const Vector &A, const Vector &B);

  friend Matrix  Pow(const Matrix &A, int NPow);
  friend double  Determ(const Matrix &A);
  friend double  Sp(const Matrix &A);
  friend Matrix  Trans(const Matrix &A);
  friend Matrix  Inverse(const Matrix &A);
  friend bool  SystemSolution(Matrix &Matr, Vector &B);
  friend bool  SystemSolution(Matrix &Matr, Matrix &B, int NB);
  friend bool  SystemSolution(Matrix &Matr, int NMatr, Matrix &B, int NB);
  };
//===============================================================================
#endif
