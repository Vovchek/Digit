#include "middle.h"
#include "MGTools\Include\Utils\Utils.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <functional>
#include <cstdint>
#include <array>

#include <cstdint>
#include <algorithm>

/**
 * @brief Remove background from a scanline segment before peak detection.
 *
 * For long segments (>70 pixels) the interval is divided into five parts.
 * The average intensity of non‑zero pixels in each part is computed.
 * Linear interpolation between these averages gives a piecewise‑linear
 * background estimate, which is subtracted from the segment.
 * For short segments (≤70 pixels) a constant background (mean of non‑zero pixels)
 * is subtracted.
 *
 * Pixels whose value falls below the estimated background are set to zero.
 *
 * @param line      Line buffer to modify in place.
 * @param leftIdx   Leftmost column index of the segment (inclusive).
 * @param rightIdx  Rightmost column index of the segment (inclusive).
 */
void fon_del_(uint8_t* line, int leftIdx, int rightIdx)
{
	const int segmentLength = rightIdx - leftIdx;
	constexpr int LONG_SEGMENT_THRESHOLD = 70;
	constexpr int NUM_PARTS = 5;

	if (segmentLength > LONG_SEGMENT_THRESHOLD)
	{
		// 1. Determine partition boundaries (inclusive end indices of each part)
		double partWidth = segmentLength / static_cast<double>(NUM_PARTS);
		int partEnds[NUM_PARTS];          // part i ends at partEnds[i] (inclusive)
		for (int i = 0; i < NUM_PARTS - 1; ++i)
		{
			partEnds[i] = static_cast<int>((i + 1) * partWidth) + leftIdx;
		}
		partEnds[NUM_PARTS - 1] = rightIdx;

		// 2. Compute average intensity of non‑zero pixels in each part
		int partMeans[NUM_PARTS] = { 0 };
		int currentPos = leftIdx;
		for (int part = 0; part < NUM_PARTS; ++part)
		{
			int sum = 0;
			int count = 0;
			while (currentPos <= partEnds[part])
			{
				uint8_t pixel = line[currentPos];
				if (pixel != 0)
				{
					sum += static_cast<int>(pixel);
					++count;
				}
				++currentPos;
			}
			if (count != 0)
			{
				// Integer average with rounding
				partMeans[part] = static_cast<int>((sum / static_cast<double>(count)));
			}
		}

		// 3. Compute midpoints of each part (used as X‑coordinates for the means)
		int midPoints[NUM_PARTS];
		for (int i = 0; i < NUM_PARTS; ++i)
		{
			midPoints[i] = static_cast<int>(i * partWidth + partWidth / 2.0 + leftIdx + 0.5);
		}

		// 4. For each interval between consecutive midpoints, fit a line
		//    slope = (mean_next - mean_prev) / (mid_next - mid_prev)
		//    intercept = mean_prev - slope * mid_prev
		double slopes[NUM_PARTS - 1];
		double intercepts[NUM_PARTS - 1];
		for (int i = 0; i < NUM_PARTS - 1; ++i)
		{
			double dx = static_cast<double>(midPoints[i + 1] - midPoints[i]);
			if (dx != 0.0)
			{
				slopes[i] = (partMeans[i + 1] - partMeans[i]) / dx;
			}
			else
			{
				slopes[i] = 0.0;   // Degenerate interval – treat as constant
			}
			intercepts[i] = partMeans[i] - slopes[i] * midPoints[i];
		}

		// 5. Apply background subtraction piecewise
		delet_u(line, leftIdx, midPoints[1], slopes[0], intercepts[0]);
		delet_u(line, midPoints[1], midPoints[2], slopes[1], intercepts[1]);  // Note: +1 to avoid overlap
		delet_u(line, midPoints[2], midPoints[3], slopes[2], intercepts[2]);
		delet_u(line, midPoints[3], rightIdx, slopes[3], intercepts[3]);
	}
	else
	{
		// Short segment: use constant background = average of non‑zero pixels
		int sum = 0;
		int count = 0;
		for (int pos = leftIdx; pos <= rightIdx; ++pos)
		{
			uint8_t pixel = line[pos];
			if (pixel != 0)
			{
				sum += static_cast<int>(pixel);
				++count;
			}
		}
		double meanBackground = 0.0;
		if (count != 0)
		{
			meanBackground = static_cast<double>(sum) / count;
		}
		delet_u(line, leftIdx, rightIdx, 0.0, meanBackground);
	}
}
/**
 * @brief Sub-pixel peak position estimation via quadratic polynomial approximation.
 *
 * Fits a quadratic model y = c0 + c1*x + c2*x^2 to the provided (x, y) samples.
 * Returns the estimated sub-pixel X-position of the peak (vertex) shifted by 0.5.
 *
 * @param n Pointer to the number of sample points. On error, set to a negative value.
 * @param x Integer X-coordinates of samples.
 * @param y Integer intensity values of samples.
 * @return Estimated center position (vertex + 0.5). On failure returns 0 and *n < 0.
 */
double approx_(int* n, int* x, int* y)
{
	const int N = *n;
	if (N < 3) {
		*n = -1;      // Not enough points
		return 0.0;
	}

	// Compute sums for the normal equations
	double Sx = 0.0, Sx2 = 0.0, Sx3 = 0.0, Sx4 = 0.0;
	double Sy = 0.0, Syx = 0.0, Syx2 = 0.0;

	for (int i = 0; i < N; ++i) {
		double xi = static_cast<double>(x[i]);
		double yi = static_cast<double>(y[i]);
		double xi2 = xi * xi;

		Sx += xi;
		Sx2 += xi2;
		Sx3 += xi2 * xi;
		Sx4 += xi2 * xi2;
		Sy += yi;
		Syx += yi * xi;
		Syx2 += yi * xi2;
	}

	// Normal equations: A * c = b
	double A[3][3] = {
		{ static_cast<double>(N), Sx,  Sx2 },
		{ Sx,                     Sx2, Sx3 },
		{ Sx2,                    Sx3, Sx4 }
	};
	double b[3] = { Sy, Syx, Syx2 };

	// Gaussian elimination with partial pivoting for the 3x3 system
	const double eps = 1e-12;
	int pivot_row = 0;

	// Forward elimination
	for (int col = 0; col < 3; ++col) {
		// Find pivot (largest absolute value in current column from pivot_row to end)
		int max_row = pivot_row;
		double max_val = std::fabs(A[pivot_row][col]);
		for (int row = pivot_row + 1; row < 3; ++row) {
			if (std::fabs(A[row][col]) > max_val) {
				max_val = std::fabs(A[row][col]);
				max_row = row;
			}
		}

		if (max_val < eps) {
			*n = -2;    // Singular matrix
			return 0.0;
		}

		// Swap rows if necessary
		if (max_row != pivot_row) {
			std::swap(A[pivot_row], A[max_row]);
			std::swap(b[pivot_row], b[max_row]);
		}

		// Eliminate below
		for (int row = pivot_row + 1; row < 3; ++row) {
			double factor = A[row][col] / A[pivot_row][col];
			for (int k = col; k < 3; ++k) {
				A[row][k] -= factor * A[pivot_row][k];
			}
			b[row] -= factor * b[pivot_row];
		}

		++pivot_row;
	}

	// Back substitution (upper triangular)
	double c[3] = { 0.0, 0.0, 0.0 };
	for (int row = 2; row >= 0; --row) {
		double sum = b[row];
		for (int k = row + 1; k < 3; ++k) {
			sum -= A[row][k] * c[k];
		}
		if (std::fabs(A[row][row]) < eps) {
			*n = -2;    // Singular (should not happen after pivoting)
			return 0.0;
		}
		c[row] = sum / A[row][row];
	}

	// Quadratic coefficient c[2] must be non‑zero for a valid peak
	if (std::fabs(c[2]) < eps) {
		*n = -3;
		return 0.0;
	}

	// Vertex of parabola: x0 = -c1 / (2*c2)
	double vertex = -c[1] / (2.0 * c[2]);

	// Return sub‑pixel position shifted by 0.5 (original convention)
	return vertex + 0.5;
}

/**
 * @brief Detects and localizes peaks in a scan line.
 *
 * Scans the line from left to right. For each contiguous run of non‑zero pixels
 * that are considered visible by the predicate, it collects up to 300 samples,
 * estimates the sub‑pixel peak position (via quadratic fit or plateau centering),
 * and adds the valid positions to the returned vector.
 *
 * @param line      Pointer to the scan line data (8‑bit intensities).
 * @param nx        Width of the line (number of pixels).
 * @param y         Row index (passed to the visibility predicate).
 * @param IsVisible Functor returning true if the pixel (x, y) belongs to the
 *                  region of interest (e.g., inside the valid scan range and
 *                  not in an exclusion zone).
 * @return std::vector<double> List of accepted sub‑pixel peak positions.
 */
std::vector<double> middle_(const std::uint8_t* line, std::size_t nx, int y,
	std::function<bool(int x, int y)> IsVisible)
{
	std::vector<double> results;
	std::size_t i = 0;

	while (i < nx)
	{
		// Find the start of a run of visible, non‑zero pixels.
		if (line[i] != 0 && IsVisible(i, y))
		{
			std::size_t start = i;
			// Advance to the end of the run.
			while (i < nx && line[i] != 0 && IsVisible(i, y))
				++i;
			std::size_t end = i;  // exclusive

			// Collect up to 300 samples from the run.
			std::array<int, 300> ax, ay;
			int maxVal = 0, firstMaxIdx = 0, n = 0;
			for (std::size_t j = start; j < end && n < 300; ++j)
			{
				int val = line[j];
				if (val > maxVal)
				{
					maxVal = val;
					firstMaxIdx = static_cast<int>(j);
				}
				ay[n] = val;
				ax[n] = static_cast<int>(j);
				++n;
			}

			if (n < 3)          // Not enough points for a reliable estimate.
				continue;

			double tmpf = 0.0;
			int half = firstMaxIdx;   // integer candidate (first maximum)

			if (n > 4)
			{
				// Use quadratic fit for longer runs.
				int n_local = n;
				tmpf = approx_(&n_local, ax.data(), ay.data());
				if (n_local < 0)       // approx failed
					continue;
				n = n_local;            // approx may have reduced the number of points

				if (tmpf > ax[0] && tmpf < ax[n - 1])
					half = std::lround(tmpf);
				else
					tmpf = half + 0.5;
			}
			else
			{
				// Short run: treat as a flat plateau.
				int kol = half;
				while (kol < static_cast<int>(end) && line[kol] == maxVal)
					++kol;
				half += (kol - half) / 2;   // integer midpoint of the plateau
				tmpf = half + 0.5;
			}

			// Reject if the estimated integer position coincides with the run boundaries.
			if (half == ax[0] || half == ax[n - 1])
				continue;

			results.push_back(tmpf);
		}
		else
		{
			++i;
		}
	}

	return results;
}


// Legacy code with direct buffer access and additional checks (e.g., exclusion zones) preserved for reference:
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

/**
 * @brief Sort an array of doubles in ascending order.
 *
 * Uses std::sort on the contiguous memory returned by CArray::GetData().
 * The function sorts in-place and preserves the original function signature.
 *
 * @param CenterFrg Array of doubles to sort (modified in place).
 */
void SortDouble(CArray<double, double>& CenterFrg)
{
	int n = CenterFrg.GetSize();
	if (n <= 1)
		return;
	double* data = CenterFrg.GetData();
	std::sort(data, data + n);
}

/**
 * @brief Sub-pixel peak position estimation via polynomial approximation.
 *
 * Fits a quadratic-like model to the provided (x, y) samples and returns
 * the estimated sub-pixel X-position of the peak (as double). The function
 * may adjust the value pointed by @p n to indicate error codes (< 0).
 *
 * @param n Pointer to the number of sample points; may be modified on error.
 * @param x Integer X-coordinates of samples.
 * @param y Integer intensity values of samples.
 * @return Estimated center position (as double). On success returns value ~ (x0 + 0.5).
 *         On failure, n is set to a negative error code and returned value is 0.
 */
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

/**
 * @brief Remove background from a scanline segment before peak detection.
 *
 * For long segments (>70 px) the function divides the interval into five
 * parts, estimates average background level in each part, fits linear
 * segments and subtracts them (zeros pixels below the estimated background).
 * For short segments the mean level is used.
 *
 * @param line Line buffer to modify in place (values below estimated background set to 0).
 * @param x Left index of the segment (inclusive).
 * @param x1 Right index of the segment (inclusive).
 */
void fon_del(uint8_t* line, int x, int x1)
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

/**
 * @brief Apply linear background model and zero samples below the model.
 *
 * For each index j in [end1, end2) the background value fon = aa*j + bb is computed.
 * If fon >= line[j] then the pixel is set to zero.
 *
 * @param line Line buffer to modify in place.
 * @param end1 Start index (inclusive).
 * @param end2 End index (exclusive).
 * @param aa Slope of the linear background.
 * @param bb Intercept of the linear background.
 */
void delet_u(uint8_t* line, int end1, int end2, double aa, double bb)
{
	for (auto j = end1; j < end2; j++)
	{
		double fon = (j * aa + bb);
		if (fon >= (double)(*(line + j)))
		{
			*(line + j) = 0;
		}
	}
}

/**
 * @brief Invert intensities on the given segment of the line.
 *
 * Replaces each pixel value v by (255 - v).
 *
 * @param line Line buffer to modify in place.
 * @param x Left index (inclusive).
 * @param x1 Right index (exclusive).
 */
void invert_line(uint8_t* line, int x, int x1)
{
	for (auto i = x; i < x1; i++) {
		line[i] = 255 - line[i];
	}
}

