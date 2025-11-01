#ifndef __VECTOR_H
#define __VECTOR_H

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <process.h>
#include "stdafx.h"
#include "include\CArrDef.h"

//===============================================================================
class Vector
  {
	protected:
    CArrayDouble Vect;

	public:
    Vector(void);
    Vector(int);
    Vector(const Vector &A) {Vect.Copy(A.Vect);}
    ~Vector();

    void SetSize(int Mv);
    void Init(double R);
		void Init(double A0, ...);
    double *GetMem(void) {return Vect.GetData();}
    int GetSize() const {return Vect.GetSize();}
    double& operator[] (int i) {return Vect[i];}
    const double operator[] (int i) const {return Vect[i];}
    Vector& operator= (const Vector &A);
    
      
    friend Vector operator+ (const Vector &A, const Vector &B);
    friend Vector operator- (const Vector &A, const Vector &B);
    friend Vector operator* (double b, const Vector &A);
    friend Vector operator* (const Vector &A, double b);
    friend Vector operator/ (const Vector &A, double b);
    friend double operator* (const Vector &A, const Vector &B);
    friend Vector operator^ (const Vector &A, const Vector &B);
    friend double Mod(const Vector &A);
    friend Vector UniMod(const Vector &A);

    friend void  Print(const Vector &A);
		friend void  Print(double);
};
//===============================================================================
#endif
