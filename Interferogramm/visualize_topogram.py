import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Read the topogram data
with open('topogram_linear.txt', 'r') as f:
    # Read first line with dimensions
    header = f.readline().strip()
    print(f"Header: {header}")
    
    # Read the matrix data
    data = np.loadtxt(f)

print(f"Matrix shape: {data.shape}")

# Create coordinate grids
rows, cols = data.shape
x = np.arange(cols)
y = np.arange(rows)
X, Y = np.meshgrid(x, y)

# Create the 3D surface plot
fig = plt.figure(figsize=(12, 9))
ax = fig.add_subplot(111, projection='3d')

# TILT ESTIMATION AND SUBTRACTION (NaN ignored)
REMOVE_TILT = True  # Set to False to disable tilt removal

if REMOVE_TILT:
    # Extract valid (non-NaN) data points
    valid_mask = ~np.isnan(data)
    valid_indices = np.where(valid_mask)
    valid_data = data[valid_mask]
    
    # Fit a plane to valid data: Z = a*X + b*Y + c
    X_valid = valid_indices[1]  # column indices
    Y_valid = valid_indices[0]  # row indices
    
    # Create design matrix for plane fitting
    A = np.column_stack([X_valid, Y_valid, np.ones(len(X_valid))])
    # Solve least squares
    coeffs = np.linalg.lstsq(A, valid_data, rcond=None)[0]
    
    # Create tilt plane
    X_mesh, Y_mesh = np.meshgrid(np.arange(cols), np.arange(rows))
    tilt_plane = coeffs[0] * X_mesh + coeffs[1] * Y_mesh + coeffs[2]
    
    # Subtract tilt (preserve NaN)
    data_detrended = data - tilt_plane
    print(f"Tilt removed: slope_x={coeffs[0]:.6f}, slope_y={coeffs[1]:.6f}, offset={coeffs[2]:.6f}")
else:
    data_detrended = data

# Replace NaN values for visualization (optional: use 0 or interpolate)
Z = np.nan_to_num(data_detrended, nan=np.nanmean(data_detrended[~np.isnan(data_detrended)]))

# Plot surface
surf = ax.plot_surface(X, Y, Z, cmap='viridis', alpha=0.8, edgecolor='none')

# Enlarge Z-axis visual scale by constraining Z-axis limits
# This makes the surface details appear exaggerated
Z_min, Z_max = np.nanmin(data_detrended), np.nanmax(data_detrended)
Z_range = Z_max - Z_min
print(f"Original Z range: {Z_min:.2f} to {Z_max:.2f} (range: {Z_range:.2f})")

# SIMPLE Z-AXIS CONTROL - adjust these directly:
Z_AXIS_MIN = Z_min      # Set custom min (e.g., 1.0)
Z_AXIS_MAX = Z_max      # Set custom max (e.g., 2.0)
ax.set_zlim([Z_AXIS_MIN, Z_AXIS_MAX])

# Also set box aspect for additional visual enhancement
Z_SCALE = 0.2  # Increase this to enlarge Z-axis visually (e.g., 2.0, 3.0, 5.0)
ax.set_box_aspect([1, 1, Z_SCALE])

# Add labels and title
ax.set_xlabel('Column Index')
ax.set_ylabel('Row Index')
ax.set_zlabel('Elevation Value')
ax.set_title('3D Surface Visualization of Topogram Matrix (509x509)')

# Add colorbar
fig.colorbar(surf, ax=ax, shrink=0.5, aspect=5)

# Show the plot
plt.tight_layout()
plt.show()
# ------------------------------------------------------------
# CENTRAL ROW PROFILE (using detrended data)
# ------------------------------------------------------------

# Determine central row index
central_row_index = rows // 2

# Extract central row from detrended data (keep NaNs)
central_row = data_detrended[central_row_index, :]

# Create new figure for 2D plot
plt.figure(figsize=(10, 5))

# Plot only valid (non-NaN) values
valid = ~np.isnan(central_row)
plt.plot(x[valid], central_row[valid])

plt.xlabel("Column Index")
plt.ylabel("Detrended Elevation")
plt.title(f"Central Row Profile (Row {central_row_index}) - Detrended")

plt.grid(True)
plt.tight_layout()
plt.show()

