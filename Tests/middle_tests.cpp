#include "pch.h"
#include <gtest/gtest.h>
#include "Utils/middle.h"
#include <vector>

// Вспомогательная обёртка для CArray<double,double> (MFC CArray используется в проекте).
// Предполагается, что в окружении сборки доступен настоящий CArray.
// Здесь используем его напрямую.
#include "MGTools/Include/Utils/BaseDataType.h" // если требуется, проект должен корректно резолвить этот include

TEST(Approx, SimpleSymmetricPeak)
{
    int n = 3;
    int x[] = {0, 1, 2};
    int y[] = {1, 2, 1};

    double res = approx(&n, x, y);
    // ожидание: вершина при x0=1, функция возвращает x0 + 0.5 => 1.5
    EXPECT_NEAR(res, 1.5, 1e-5);
    // n не должен быть негативным (ошибка)
    EXPECT_GT(n, 0);
}

TEST(SortDouble, SortsAscending)
{
    CArray<double, double> arr;
    arr.Add(3.0);
    arr.Add(1.0);
    arr.Add(2.0);

    SortDouble(arr);

    ASSERT_EQ(arr.GetSize(), 3);
    EXPECT_DOUBLE_EQ(arr[0], 1.0);
    EXPECT_DOUBLE_EQ(arr[1], 2.0);
    EXPECT_DOUBLE_EQ(arr[2], 3.0);
}

TEST(DeletU, ZeroesValuesBelowBgModel)
{
    std::vector<unsigned char> buf = {10, 50, 100};
    // применяем фон = 60 на диапазон [0,3) -> элементы с value <= 60 станут 0
    delet_u(buf.data(), 0, 3, 0.0, 60.0);

    EXPECT_EQ(buf[0], 0);
    EXPECT_EQ(buf[1], 0);
    EXPECT_EQ(buf[2], 100);
}

TEST(InvertLine, InvertsByteValues)
{
    std::vector<unsigned char> buf = {0, 100, 255};
    invert_line(buf.data(), 0, 3);

    // 0 -> 255, 100 -> 155, 255 -> 0
    EXPECT_EQ(buf[0], static_cast<unsigned char>(255));
    EXPECT_EQ(buf[1], static_cast<unsigned char>(155));
    EXPECT_EQ(buf[2], static_cast<unsigned char>(0));
}

TEST(Middle, SinglePeakDetected)
{
    const int nx = 10;
    const int ny = 1;
    const int y = 0;

    // подготовим линию: все нули, на позициях 2,3,4 -- 1,2,1 (пик в 3)
    std::vector<unsigned char> line(nx, 0);
    line[2] = 1;
    line[3] = 2;
    line[4] = 1;

    // buf_line: один ряд с {start, end, int2, int3}
    int** buf_line = (int**) malloc(sizeof(int*));
    buf_line[0] = (int*) malloc(sizeof(int) * 4);
    buf_line[0][0] = -1;   // loop starts from buf_line[y][0] + 1 -> 0
    buf_line[0][1] = nx;   // end exclusive in outer loop condition
    buf_line[0][2] = -1;   // disable exclusion check in middle
    buf_line[0][3] = -1;

    CArray<double, double> centers;
    int nnpolos = 0;

    middle(line.data(), nx, ny, y, buf_line, centers, nnpolos);

    // ожидание: найден один пик, tmpf = half + 0.5; half == 3 -> tmpf == 3.5
    EXPECT_EQ(nnpolos, 1);
    ASSERT_EQ(centers.GetSize(), 1);
    EXPECT_NEAR(centers[0], 3.5, 1e-6);

    free(buf_line[0]);
    free(buf_line);
}
