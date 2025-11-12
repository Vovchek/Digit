#include <math.h>
#include "matin1.h"
//=====================================================================
/**
 * @brief Solves systems of linear equations using Gaussian elimination with partial pivoting
 * 
 * This function implements Gaussian elimination with partial pivoting to solve systems of
 * linear equations Ax = b. It can solve multiple systems simultaneously and computes the
 * determinant of the coefficient matrix as a byproduct.
 * 
 * @param A Pointer to the augmented matrix containing both coefficient matrix and RHS vectors
 * @param N1 Number of rows (equations) in the system
 * @param N2 Number of right-hand side vectors (can solve multiple systems simultaneously)
 * @param NeError Output error flag: 1 = success, 0 = system is incompatible/singular
 * @param Determ Output determinant of the coefficient matrix
 * 
 * ## Matrix Layout
 * The input matrix A is organized as an augmented matrix:
 * @code
 * [a₁₁ a₁₂ ... a₁ₙ | b₁₁ b₁₂ ... b₁ₙ₂]
 * [a₂₁ a₂₂ ... a₂ₙ | b₂₁ b₂₂ ... b₂ₙ₂]
 * [... ... ... ... | ... ... ... ...]
 * [aₙ₁ aₙ₂ ... aₙₙ | bₙ₁ bₙ₂ ... bₙₙ₂]
 * @endcode
 * 
 * ## Algorithm Steps
 * 1. **Forward Elimination**: For each column (Main = 1 to N):
 *    - **Pivot Selection**: Finds largest absolute value in current column below diagonal
 *    - **Singularity Check**: Returns error if pivot is zero (singular matrix)
 *    - **Row Swapping**: Swaps rows to bring pivot to diagonal position if needed
 *    - **Determinant Update**: Negates determinant when rows are swapped
 *    - **Elimination**: Transforms pivot column and eliminates entries below pivot
 * 
 * 2. **Back Substitution**: Implicit through the elimination process creating upper triangular form
 * 
 * 3. **Row Rearrangement**: Undoes row swaps to restore correct solution order
 * 
 * ## Key Features
 * - **Partial Pivoting**: Improves numerical stability by selecting largest available pivot
 * - **Multiple RHS**: Solves Ax₁ = b₁, Ax₂ = b₂, ... simultaneously
 * - **Determinant Calculation**: Computes det(A) during elimination
 * - **In-place Operation**: Modifies input matrix directly for memory efficiency
 * - **Error Detection**: Identifies singular/incompatible systems
 * 
 * @note After execution, the last N2 columns of matrix A contain the solutions x₁, x₂, ..., xₙ₂
 * @warning The input matrix A is modified in-place during computation
 */
void Matin1(double *A, int N1, int N2, int &NeError, double &Determ)
{
  int Dim, Emat, Pivcol, Pivc_L1, Pivc_L2, Lpiv, Icol, I3, I2, I1, Jcol, Main;
  int i;
  double Pivot, Swap;
  int *Index = new int[N1];
  for(int j=0; j < N1; j++) Index[j] = 0;
  double Deter = 1.;
  int N = N1;
  Emat = N + N2;
  Dim = N1;
  int Nmin1 = N-1;
  
  // Initialize column pointer for matrix traversal (1-based indexing)
  Pivcol = 1 - Dim;
  
  // Main elimination loop - process each column
  for(Main = 1; Main < N+1; Main++) {
     Pivot = 0.;
     Pivcol = Pivcol + Dim;
     
     // Pivot selection: find largest absolute value in current column
     Pivc_L1=Pivcol + Main - 1;
     Pivc_L2=Pivcol + Nmin1;
     for(i = Pivc_L1; i < Pivc_L2+1; i++) {
        if((fabs(A[i-1]) - fabs(Pivot)) > 0.) {
          Pivot = A[i-1];
          Lpiv = i;
        }
     }
        // Check for singular matrix (zero pivot)
        if(!Pivot) {
          NeError = 0;
          Determ = Deter;
          if(Index) delete[] Index;
          return;
        }
     
     // Determine if row swapping is needed for pivoting
     Icol = Lpiv - Pivcol + 1;
     Index[Main-1] = Icol;
     if((Icol - Main) > 0.) {
       // Row swap needed - negate determinant
       Deter = -Deter;
       
       // Calculate pointers for row swapping
       Icol = Icol - Dim;
       I3 = Main - Dim;
       for(i=1; i < Emat+1; i++) {
          Icol = Icol + Dim;
          I3 = I3 + Dim;
          Swap = A[I3-1];
          A[I3-1] = A[Icol-1];
          A[Icol-1] = Swap;
       }
     }
     
     // Store reciprocal of pivot for efficiency
     Pivot = 1. / Pivot;
     
     // Transform pivot column elements
     I3 = Pivcol + Nmin1;
     for(i = Pivcol; i < I3+1; i++) A[i-1] = -A[i-1] * Pivot;
     A[Pivc_L1-1] = Pivot;
     
     // Eliminate entries in other columns using transformed pivot row
      I1=Main-Dim;  // Pointer to pivot row elements
      Icol=1-Dim;   // General column pointer
      for(i = 1; i < Emat+1; i++) {
         Icol = Icol + Dim;
         I1 = I1 + Dim;
         
         // Skip the pivot column itself
         if(i - Main) {
           Jcol = Icol + Nmin1;
           Swap = A[I1-1];
           I3 = Pivcol - 1;
           for(I2 = Icol; I2 < Jcol+1; I2++) {
              I3++;
              A[I2-1] = A[I2-1] + Swap*A[I3-1];
           }
           A[I1-1] = Swap * Pivot;
         }
      }
  }
  
  // Rearrange matrix rows to undo pivoting and get correct solution order
  for(I1=1; I1 < N+1; I1++) {
     Main = N + 1 - I1;
     Lpiv = Index[Main-1];
     if(Lpiv - Main) {
       Icol = (Lpiv-1) * Dim+1;
       Jcol = Icol + Nmin1;
       Pivcol = Main  * Dim+1 - Icol;
       for(I2 = Icol-1; I2 < Jcol; I2++) {
          I3 = I2 + Pivcol;
          Swap = A[I2];
          A[I2] = A[I3];
          A[I3] = Swap;
       }
     }
  }
  
  // Set output values and cleanup
  Determ = Deter;
  NeError = 1;
  if(Index) delete[] Index;
  return;
}
//=====================================================================