#if !defined(_CONTUOR_H__)
#define _CONTUOR_H__

#include    <stdio.h>
#include    <stdlib.h>
#include    <math.h>
#include    <malloc.h>
#include    <memory.h>
#include    <string.h>
#include    <time.h>
#include    "stdafx.h"
#include    "MGToolsExport.h"

#define MINCELLS 30

#define     NaN         0xFFFFFFFF
#define     isnanf(x)   ((x) == NaN)

typedef unsigned    short   ushort;
typedef unsigned    char    uchar;

#define DEFAULT_LEVELS  16

/* Mnemonics for contour line bearings  */
#define EAST        0
#define NORTH       1
#define WEST        2
#define SOUTH       3

/* Mnemonics for relative data point positions  */
#define SAME        0
#define NEXT        1
#define OPPOSITE    2
#define ADJACENT    3

/* Bit-mapped information in 'map' field.  */
#define EW_MAP      0x01
#define NS_MAP      0x02

typedef struct
  {
  double   x, y;
  } ISO_POINT;

typedef struct
  {
  short      dim_x;          /*  dimensions of grid array... */
  short      dim_y;
  double    *px;
  double    *py;
  double     max_value;      /*  statistics on the data...   */
  double     min_value;
  double     mean;
  short      contour_mode;   /*  control variable            */
  int        n_levels;       /*  number of levels            */
  double    *levels;         /*  levels of isolines          */
  double    *data;           /*  pointer to grid data        */
  char      *map;            /*  pointer to "in-use" map     */
  ISO_POINT *list;           /*  used by 'Polyline()'        */
  ushort     count;
  void (*polyCallBack)(int, ISO_POINT *, double , int );
  } GRID;

typedef struct
  {
  short   x;
  short   y;
  uchar   bearing;
  }IS_POINT;

#define MXY_to_L(g,x,y)     ((ushort) (y) * (g)->dim_x + (ushort) (x) + 1)
#define XY_to_L(g,x,y)      ((ushort) (y) * (g)->dim_x + (ushort) (x))

/* Contour generation. */
static void startLine(GRID *grid);
static void startEdge(GRID *grid, int ilevel, uchar bearing);
static void startInterior(GRID *grid, int ilevel);
static void drawLine(GRID *grid, IS_POINT *point, int ilevel);
static void markInUse(GRID *grid, IS_POINT *point, uchar face);
static uchar faceInUse(GRID *grid, IS_POINT *point, uchar face);
static void initPoint(GRID *grid);
static void lastPoint(GRID *grid, int ilevel);
static uchar savePoint(GRID *grid, IS_POINT *point, int ilevel);
static double getDataPoint(GRID *grid, IS_POINT *point, uchar corner);

void MGTOOLS_API Contour(double *data, int dim_x, int dim_y, double *px, double *py, int n_levels, double *levels,
             void (*polyCB)(int, ISO_POINT *, double , int ));

#endif
