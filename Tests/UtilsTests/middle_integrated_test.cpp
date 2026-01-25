#include "stdafx.h" // или "pch.h", как у вашего тест-проекта
#include <gtest/gtest.h>
#include <cmath>

// Подключаем оригинальный заголовок — он объявляет все функции, которые мы тестируем
#include "Utils/middle.h" // путь относительно Tests; скорректируйте при необходимости

// Тест для функции middle: создаём простой сигнал с одним локальным пиком и проверяем,
// что в CenterFrg добавлен центр пика и nnpolos увеличен.
TEST(MiddleIntegrated, FindsCenterOfSimple3PixelPeak)
{
    const int LEN = 40;
    unsigned char line[LEN] = {0};

    // поместим пик на позициях 10,11,12: values {1,3,1}
    line[10] = 1;
    line[11] = 3;
    line[12] = 1;

    // Подготовим buf_line: одна строка y=0, buf_line[0] = {left, right, excl_left, excl_right}
    int* row0 = new int[4];
    row0[0] = 9;   // left boundary (buf_line[y][0])
    row0[1] = 13;  // right boundary (buf_line[y][1])
    row0[2] = -1;  // exclusion left (buf_line[y][2]) - отключено
    row0[3] = 0;   // exclusion right (buf_line[y][3])
    int** buf_line = new int*[1];
    buf_line[0] = row0;

    // CArray из вашего проекта
    CArray<double, double> CenterFrg;
    int nnpolos = 0;

    // вызов реальной функции
    middle(line, /*nx*/ 0, /*ny*/ 1, /*y*/ 0, buf_line, CenterFrg, nnpolos);

    // ожидания:
    // Для симметричного пика максимальный элемент в позиции 11 -> tmpf будет half + 0.5 => 11.5
    ASSERT_EQ(nnpolos, 1);
    ASSERT_EQ(CenterFrg.GetSize(), 1);
    double center = CenterFrg[0];
    EXPECT_NEAR(center, 11.5, 1e-6);

    delete[] buf_line;
    delete[] row0;
}

// Интеграционные тесты для прочих функций (approx, delet_u, invert_line, fon_del, SortDouble)

// approx: простая симметрия (максимум в центре)
TEST(MiddleIntegrated, ApproxReturnsCenterForSymmetricTriplet)
{
    int xs[3] = {0, 1, 2};
    int ys[3] = {3, 4, 3}; // максимум при x=1
    int n = 3;
    double res = approx(&n, xs, ys);
    EXPECT_GT(res, 1.0);
    EXPECT_LT(res, 2.0);
}

// delet_u: проверяем обнуление значений <= фон
TEST(MiddleIntegrated, DeletUZeroesValuesBelowBackground)
{
    const int LEN = 10;
    unsigned char line[LEN];
    for (int i = 0; i < LEN; ++i) line[i] = static_cast<unsigned char>(i + 1); // 1..10

    // фон = 5 -> все значения <=5 обнуляются
    delet_u(line, 0, LEN, 0.0, 5.0);

    for (int i = 0; i < LEN; ++i)
    {
        if (i + 1 <= 5) EXPECT_EQ(line[i], 0);
        else EXPECT_EQ(line[i], static_cast<unsigned char>(i + 1));
    }
}

// invert_line: инверсия байтов
TEST(MiddleIntegrated, InvertLineInvertsBytes)
{
    const int LEN = 6;
    unsigned char line[LEN] = {0, 10, 127, 128, 200, 255};
    invert_line(line, 0, LEN);
    unsigned char expected[LEN] = {255, 245, 128, 127, 55, 0};
    for (int i = 0; i < LEN; ++i)
        EXPECT_EQ(line[i], expected[i]);
}

// fon_del: короткий сегмент -> используется средний фон
TEST(MiddleIntegrated, FonDelShortSegmentUsesMean)
{
    const int LEN = 8;
    unsigned char line[LEN] = {0, 10, 20, 30, 0, 5, 15, 25};
    // mean non-zero = (10+20+30+5+15+25)/6 = 17.5 -> rounded 18
    fon_del(line, 0, LEN - 1);
    // ожидаем, что значения <= 18 обнулены (10,5,15)
    EXPECT_EQ(line[1], 0);   // 10 -> 0
    EXPECT_EQ(line[2], 20);  // 20 > 18
    EXPECT_EQ(line[3], 30);
    EXPECT_EQ(line[5], 0);   // 5 -> 0
    EXPECT_EQ(line[6], 0);   // 15 -> 0
}

// SortDouble: используем CArray (оригинал)
TEST(MiddleIntegrated, SortDoubleSortsArray)
{
    CArray<double, double> arr;
    arr.Add(5.0);
    arr.Add(1.0);
    arr.Add(-2.0);
    arr.Add(3.5);
    // вызываем функцию SortDouble (реальная)
    SortDouble(arr);
    for (int i = 1; i < arr.GetSize(); ++i)
        EXPECT_LE(arr[i - 1], arr[i]);
}