# Matrix Transpose Bug Fix

## Issue Summary

There was a **parameter order mismatch** in the `Matrix::Trans()` function that caused incorrect matrix dimensions after transposition.

## Root Cause

### Matrix Class Design
The `Matrix` class uses the following conventions:
- **Constructor signature**: `Matrix(int My, int Mx)` where `My` = rows, `Mx` = columns
- **Internal storage**: 
  - `Nx` = number of columns (X-dimension)
  - `Ny` = number of rows (Y-dimension)
- **Indexing**: `operator()(int iy, int ix)` where `iy` = row, `ix` = column
- **Storage layout**: Row-major order: `Matr[iy*Nx+ix]`

### The Bug

In `Trans()` function (line 333-343):

**Before (INCORRECT)**:
```cpp
Matrix Trans(const Matrix &A)
  {
  int Mx = A.GetSizeX();  // Mx = columns of A
  int My = A.GetSizeY();  // My = rows of A
  Matrix C(My,Mx);        // ? BUG! Creates matrix with SAME dimensions as A
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(iy,ix) = A(ix,iy);  // Correctly swaps indices
		}
	return C;
  }
```

**Problem**: 
- If matrix A is 3?5 (3 rows, 5 columns):
  - `My` = 3, `Mx` = 5
  - `Matrix C(My,Mx)` = `Matrix C(3,5)` = **3?5 matrix**
  - But transpose of 3?5 should be **5?3**!

The bug is that while the loop correctly swaps the indices `C(iy,ix) = A(ix,iy)`, the result matrix `C` is created with the **wrong dimensions**.

## Fix Applied

**After (CORRECT)**:
```cpp
Matrix Trans(const Matrix &A)
  {
  int Mx = A.GetSizeX();  // Mx = columns of A
  int My = A.GetSizeY();  // My = rows of A
  Matrix C(Mx,My);        // ? FIXED! Creates matrix with swapped dimensions
  for (int iy = 0; iy < My; iy++) 
    {
    for (int ix = 0; ix < Mx; ix++) 
      C(ix,iy) = A(iy,ix);  // Swap indices for transpose
		}
	return C;
  }
```

**Solution**:
- Changed `Matrix C(My,Mx)` to `Matrix C(Mx,My)`
- Now for a 3?5 matrix:
  - `My` = 3, `Mx` = 5
  - `Matrix C(Mx,My)` = `Matrix C(5,3)` = **5?3 matrix** ? Correct!

## Impact

This bug would cause:
1. **Memory corruption** when accessing transposed matrices with non-square dimensions
2. **Incorrect calculations** in any algorithm using matrix transpose
3. **Crashes** when the transposed matrix dimensions are used for further operations

## Verification

Tested with a 3?5 matrix:
```
Original A (3?5):      Transpose C (5?3):
[1 2 3 4 5]           [1 6 11]
[6 7 8 9 10]    ?     [2 7 12]
[11 12 13 14 15]      [3 8 13]
                      [4 9 14]
                      [5 10 15]
```

## Files Modified

- `InterfSolver/Tools/Matrix.cpp` - Line ~339: Changed `Matrix C(My,Mx)` to `Matrix C(Mx,My)`

## Related Code Review

All other matrix operations correctly use the `(rows, columns)` parameter order:
- `operator+`, `operator-`, `operator*` ?
- `operator^`, `Pow()` ?  
- `Init()`, `SetSize()` ?
- `Extract()` has intentional parameter swap for extraction bounds ?
