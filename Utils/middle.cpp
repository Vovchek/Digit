#include "middle.h"
#include "MGTools\Include\Utils\Utils.h"
#include <cmath>
#include <algorithm>

void middle(unsigned char* line, int nx, int ny, int y, int** buf_line,
	CArray<double, double>& CenterFrg, int& nnpolos)
{
	int     ax[300], ay[300];
	double   tmpf = 0.;
	int     i, j, max, half = 0, kol, n;
	for (i = buf_line[y][0] + 1; i < buf_line[y][1]; i++)
	{
		if (*(line + i) != 0)
		{
			for (j = i; *(line + i) != 0 && i <= buf_line[y][1]; i++);
			for (max = n = 0; j < i && n < 300; j++)
			{
				if (max < *(line + j))
				{
					max = *(line + j);
					half = j;
				}
				ay[n] = *(line + j);
				ax[n++] = j;
			}
			if (n < 3) continue;
			if (n > 4)
			{
				tmpf = approx(&n, ax, ay);
				if (n < 0) continue;
			}
			else
			{
				for (kol = half; max == *(line + kol); kol++);
				half += (kol - half) >> 1;
			}

			if (tmpf > ax[0] && tmpf < ax[n - 1])  half = std::lround(tmpf);
			else    tmpf = half + .5;
			if (half >= buf_line[y][1] - 1 || half <= buf_line[y][0] + 1) continue;
			//               if(*(line-nx+half)==0 || *(line+nx+half)==0) continue;
			if (buf_line[y][2] != -1 &&
				(half >= buf_line[y][2] - 1 && half <= buf_line[y][3] + 1)) continue;
			if (half == ax[0] || half == ax[n - 1]) continue;

			//            outpixel962((int)(half*kw),y_scr,14);
			//            outpixel962((int)(half*kw-1),y_scr,14);
			CenterFrg.Add(tmpf);
			//            yykoord[nnpolos]=y_scr;
			nnpolos++;
		}
	}
}

void SortDouble(CArray<double, double>& CenterFrg)
{
	int n = CenterFrg.GetSize();
	if (n <= 1)
		return;
	double* data = CenterFrg.GetData();
	std::sort(data, data + n);
}

double approx(int* n, int* x, int* y)
{
	int   i, num_dot;
	double a[3][3], b[3], c[3], a_t, b_t, m;
	float x0;

	b[0] = 0.0;
	b[1] = 0.0;
	b[2] = 0.0;

	a[0][0] = 0.0;
	a[0][1] = 0.0;
	a[0][2] = 0.0;
	a[1][0] = 0.0;
	a[1][1] = 0.0;
	a[1][2] = 0.0;
	a[2][0] = 0.0;
	a[2][1] = 0.0;
	a[2][2] = 0.0;

	num_dot = *n;
	for (i = 0; i < num_dot; i++)
	{
		a_t = x[i];
		b_t = y[i];
		a[0][1] += a_t;       //Ex
		b[0] += b_t;          //Ey

		a_t *= x[i];
		b_t *= x[i];
		a[0][2] += a_t;       //Ex**2
		b[1] += b_t;          //Eyx

		a_t *= x[i];
		b_t *= x[i];
		a[1][2] += a_t;       //Ex**3
		b[2] += b_t;          //Eyx**2

		a_t *= x[i];
		a[2][2] += a_t;       //Ex**4
	}
	a[0][0] = num_dot;
	a[1][0] = a[0][1];
	a[2][0] = a[1][1] = a[0][2];
	a[2][1] = a[1][2];

	m = a[1][0] / a[0][0];
	a[1][1] -= m * a[0][1];
	a[1][2] -= m * a[0][2];
	b[1] -= m * b[0];

	m = a[2][0] / a[0][0];
	a[2][1] -= m * a[0][1];
	a[2][2] -= m * a[0][2];
	b[2] -= m * b[0];

	if (a[1][1] == 0)
		if (a[2][1] == 0)
		{
			*n = -2;
			return 0;
		}
		else
		{
			m = a[1][1]; a[1][1] = a[2][1]; a[2][1] = m;
			m = a[1][2]; a[1][2] = a[2][2]; a[2][2] = m;
			m = b[2]; b[2] = b[1]; b[1] = m;
		}
	if (a[1][1] == 0) { *n = -3; return 0; }
	m = a[2][1] / a[1][1];
	a[2][2] -= m * a[1][2];
	b[2] -= m * b[1];

	if (b[2] == a[2][2]) c[2] = 1;
	else
	{
		if (a[2][2] == 0) { *n = -3; return 0; }
		c[2] = b[2] / a[2][2];
	}
	if (c[2] == 0) { *n = -3; return 0; }
	m = b[1] - a[1][2] * c[2];
	if (m == a[1][1]) c[1] = 1;
	else c[1] = m / a[1][1];
	//m=b[0]-a[0][1]*c[1]-a[0][2]*c[2];
	//if(m==a[0][0]) c[0]=1;
	//else c[0]=m/a[0][0];

	x0 = static_cast<float>(-c[1] / (2 * c[2]));
	return x0 + 0.5;
}

/*
   Óäàëåíèå ôîíà ñòðîêè ïåðåä îïðåäåëåíèåì öåíòðîâ ïîëîñ
*/
void fon_del(unsigned char* line, int x, int x1)
{
	int     stop[5];
	int     sred[5];
	double   a[4], b[4];
	double   tmpf;
	int     i, j, k;
	unsigned char  tmpc;
	if (x1 - x > 70)
	{
		tmpf = (x1 - x) / 5.0;
		for (i = 0; i < 4; i++) stop[i] = (int)((i + 1) * tmpf + x);
		stop[4] = x1;
		for (j = x, i = 0; i < 5; i++)
		{
			sred[i] = 0;
			for (k = 0; j <= stop[i]; j++)
			{
				tmpc = *(line + j);
				if (tmpc != 0)
				{
					k++;
					sred[i] += (int)tmpc;
				}
			}
			if (k != 0)      sred[i] = (int)(sred[i] / k + 0.5);
		}

		for (i = 0; i < 5; i++) stop[i] = (int)(i * tmpf + tmpf / 2 + x + 0.5);
		for (i = 0; i < 4; i++)
		{
			a[i] = (double)(sred[i + 1] - sred[i]) / (double)(stop[i + 1] - stop[i]);
			b[i] = (double)sred[i] - a[i] * stop[i];
		}
		delet_u(line, x, stop[1], (double)a[0], (double)b[0]);
		delet_u(line, stop[1], stop[2], (double)a[1], (double)b[1]);
		delet_u(line, stop[2], stop[3], (double)a[2], (double)b[2]);
		delet_u(line, stop[3], x1, (double)a[3], (double)b[3]);

	}
	else
	{
		sred[0] = 0;
		for (k = 0, j = x; j <= x1; j++)
		{
			tmpc = *(line + j);
			if (tmpc != 0)
			{
				k++;
				sred[0] += (int)tmpc;
			}
		}
		if (k != 0)      sred[0] = (int)(sred[0] / k + 0.5);
		delet_u(line, x, x1, 0.0, (double)sred[0]);
	}
}
//=============================================================
void delet_u(unsigned char* line, int end1, int end2, double aa, double bb)
{
	double   fon;
	int     j;
	for (j = end1; j < end2; j++)
	{
		fon = (j * aa + bb);
		if (fon >= (double)(*(line + j)))
		{
			*(line + j) = 0;
		}
	}
}
//=============================================================
void invert_line(unsigned char* line, int x, int x1)
{
	for (long int i = x; i < x1; i++) {
		unsigned char cc1 = (unsigned char)line[i];
		int sw = cc1;
		sw = 255 - sw;
		if (sw < 0)sw = 0;
		else if (sw > 255) sw = 255;
		line[i] = (char)sw;
	}
}

/*
int utug(int sech, int x, int x1)
{
int     pick,i,err=0;
for(i=x; i<=x1; i++)
		{
		pick=0;
		if(bufer[sech][i-1] != 0) pick++;
		if(bufer[sech][i+1] != 0) pick++;
		if(bufer[sech-1][i] != 0) pick++;
		if(bufer[sech+1][i] != 0) pick++;
		if(bufer[sech][i]==0)
				{
				if(pick >=3)
						{
						bufer[sech][i]=(bufer[sech][i-1]+bufer[sech][i+1]+bufer[sech-1][i]+bufer[sech+1][i])/pick;
						err++;
						}
				}
		else
				{
				if(pick <= 1)
						{
						bufer[sech][i]=0;
						err++;
						}
				}
		}
return err;
}
//**********************************************************************
//********* ”¨«ìâà æ¨ï ¯® ¤¢ã¬ â®çª ¬ (á«¥¢  ¨ á¯à ¢ )  ****************
//**********************************************************************
int utug1(int sech, int x, int x1)
{
int     pick,i,err=0;
for(i=x; i<=x1; i++)
		{
		pick=0;
		if(bufer[sech-1][i] != 0) pick++;
		if(bufer[sech+1][i] != 0) pick++;
		if(bufer[sech][i]==0)
				{
				if(pick == 2)
						{
						bufer[sech][i]=(bufer[sech-1][i]+bufer[sech+1][i])>>1;
						err++;
						}
				}
		else
				{
				if(pick == 0)
						{
						bufer[sech][i]=0;
						err++;
						}
				}
		}
return err;
}
//**********************************************************************
*/
