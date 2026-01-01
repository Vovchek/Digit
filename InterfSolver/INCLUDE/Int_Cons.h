#ifndef _INT_CONS_H_
#define _INT_CONS_H_

enum KEY_DEF { UNDEFINED, DEFINED };
enum KEY_INP { NOINP, YESINP };

#define MINV(v1, v2) ((v1 < v2) ? v1:v2)
#define MAXV(v1, v2) ((v1 > v2) ? v1:v2)
#define SIGN(v) ((v >= 0) ? 1. : -1.)

#define  PRECISION 0.00000001
#define  HIGH_PRECISION 1.e-15

#define  DAT 0
#define  ZAP 1
#define  POL 2
#define  FRN 3
#define  MTR 4
#define  SEC 5

#define  WAV 0
#define  MKM 1

#define  A_GLOBAL   0
#define  A_LOCAL    1
#define  A_SECTION  2

#define  NONE   0
#define  PLANE  1
#define  SPHERE 2

#define  E_ROUND    0
#define  E_ELLIPSE  1
#define  E_RECT     2

#define  INTERNAL 0
#define  EXTERNAL 1

#define  MEASURING   0
#define  NORMALISED  1

#define  CALC_ERROR 0
#define  CALC_OK    1

#define  N_WMATR 151
#define  N_TAB 11
#define  N_CONT 501

#define  N_ELL 10

#define  EPSR     1.E-5
#define  EPSA     1.E-5
#define  E18     -1.E18
#define  E17     -1.E17
#define  E10      1.E10
#define  E3       1.E-3
#define  E4       1.E-4
#define  PI	      3.14159265359
#define  PI2      6.28318530717958647692
#define  PIG      180.
#define  PIG2     360.
#define  GRD_RD   0.01745329251994329576
#define  RD_GRD   57.2957795130823208767
#define  MIN      60.
#define  THESHOLD 1.e-3
#endif
