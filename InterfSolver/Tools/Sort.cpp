#include "Sort.h"
#include <algorithm>

//=========================================================================
// Replace manual selection-sort with std::sort for clarity and performance.
void SortIncrease(CArrayDouble& ArrX)
{
    int NArr = ArrX.GetSize();
    if (NArr <= 1)
        return;

    double* data = ArrX.GetData();
    std::sort(data, data + NArr);
}
//=========================================================================