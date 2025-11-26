#include "stdafx.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

/*
 Тестовые (локальные) версии функций из Utils/middle.cpp.
 Они копируют логику оригинала, но используют std::vector вместо CArray
 и обычные массивы вместо MFC-зависимостей, чтобы тесты можно было
 запустить отдельно от MFC/пре-компилированных заголовков.
*/

// approx (копия логики, адаптирована для std::vector)
double approx_under_test(int* n, int* x, int* y)
{
    int i, num_dot;
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

    x0 = static_cast<float>(-c[1] / (2 * c[2]));
    return x0 + 0.5;
}

// delet_u (копия логики)
void delet_u_under_test(unsigned char* line, int end1, int end2, double aa, double bb)
{
    double fon;
    int j;
    for (j = end1; j < end2; j++)
    {
        fon = (j * aa + bb);
        if (fon >= (double)(*(line + j)))
        {
            *(line + j) = 0;
        }
    }
}

// invert_line (копия логики)
void invert_line_under_test(unsigned char* line, int x, int x1)
{
    for (long int i = x; i < x1; i++) {
        unsigned char cc1 = (unsigned char)line[i];
        int sw = cc1;
        sw = 255 - sw;
        if (sw < 0) sw = 0;
        else if (sw > 255) sw = 255;
        line[i] = (char)sw;
    }
}

// fon_del (короткий вариант из оригинала: использует delet_u)
void fon_del_under_test(unsigned char* line, int x, int x1)
{
    int sred0 = 0;
    int k = 0;
    unsigned char tmpc;
    for (int j = x; j <= x1; j++)
    {
        tmpc = *(line + j);
        if (tmpc != 0)
        {
            k++;
            sred0 += (int)tmpc;
        }
    }
    if (k != 0)      sred0 = (int)(sred0 / k + 0.5);
    delet_u_under_test(line, x, x1, 0.0, (double)sred0);
}

/* ===========================
   ТЕСТЫ
   =========================== */

TEST(ApproxTests, SimpleParabolaSymmetricPeak)
{
    // Построим симметричный пик: y = - (x-1)^2 + 4  на x = {0,1,2}
    int xs[3] = {0, 1, 2};
    int ys[3] = {3, 4, 3}; // смещение, максимум в центре
    int n = 3;
    double center = approx_under_test(&n, xs, ys);
    // Ожидаем, что оценка центра будет около 1.0 + 0.5 = 1.5 (в реализации возвращается x0+0.5)
    EXPECT_GT(center, 1.0);
    EXPECT_LT(center, 2.0);
}

TEST(ApproxTests, DegenerateTooFewPointsReturnsError)
{
    int xs[1] = {0};
    int ys[1] = {10};
    int n = 1;
    double res = approx_under_test(&n, xs, ys);
    // Для одной точки алгоритм должен пометить n отрицательным или вернуть 0.
    EXPECT_TRUE(n < 0 || res == 0.0);
}

TEST(DeletUTests, ZerosBelowLinearBackground)
{
    const int LEN = 10;
    unsigned char line[LEN];
    for (int i = 0; i < LEN; ++i) line[i] = (unsigned char)(i + 1); // 1..10
    // Пусть фон = 5 (aa=0, bb=5) => все значения <=5 обнуляются
    delet_u_under_test(line, 0, LEN, 0.0, 5.0);
    for (int i = 0; i < LEN; ++i)
    {
        if (i + 1 <= 5) EXPECT_EQ(line[i], 0);
        else EXPECT_EQ(line[i], (unsigned char)(i + 1));
    }
}

TEST(InvertLineTests, InvertsBytesCorrectly)
{
    const int LEN = 6;
    unsigned char line[LEN];
    unsigned char orig[LEN] = {0, 10, 127, 128, 200, 255};
    for (int i = 0; i < LEN; ++i) line[i] = orig[i];
    invert_line_under_test(line, 0, LEN);
    for (int i = 0; i < LEN; ++i)
    {
        unsigned char expected = (unsigned char)(255 - orig[i]);
        EXPECT_EQ(line[i], expected);
    }
}

TEST(FonDelTests, ShortSegmentUsesMean)
{
    const int LEN = 8;
    unsigned char line[LEN];
    // набор интенсивностей: {0, 10, 20, 30, 0, 5, 15, 25}
    unsigned char init[LEN] = {0, 10, 20, 30, 0, 5, 15, 25};
    for (int i = 0; i < LEN; ++i) line[i] = init[i];
    // mean over nonzero = (10+20+30+5+15+25)/6 = 17.5 -> округл. 18
    fon_del_under_test(line, 0, LEN - 1);
    // Ожидаем, что элементы <=18 обнулятся (10, 5, 15)
    EXPECT_EQ(line[0], 0);
    EXPECT_EQ(line[1], 0); // 10 -> 0
    EXPECT_EQ(line[2], (unsigned char)20);
    EXPECT_EQ(line[3], (unsigned char)30);
    EXPECT_EQ(line[4], 0);
    EXPECT_EQ(line[5], 0);
    EXPECT_EQ(line[6], 0);
    EXPECT_EQ(line[7], (unsigned char)25);
}

/* Запуск тестов: стандартный main GoogleTest */
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}