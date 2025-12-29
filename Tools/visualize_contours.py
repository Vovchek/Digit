"""
Visualize contour points from CalcContour debug output
Reads points from C:\temp\contour_points.txt and displays them
"""

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection

def parse_contour_file(filename):
    """
    Parse the contour debug file and return a list of contours.
    Each contour is a dict with 'id', 'type', 'points', and 'coords' (numpy array).
    """
    contours = []
    current_contour = None
    
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            
            # Skip empty lines
            if not line:
                if current_contour and len(current_contour['coords']) > 0:
                    # Convert list to numpy array
                    current_contour['coords'] = np.array(current_contour['coords'])
                    contours.append(current_contour)
                    current_contour = None
                continue
            
            # Parse contour header: "Contour 0 (Type: 1, Points: 502):"
            if line.startswith('Contour'):
                parts = line.split()
                contour_id = int(parts[1])
                
                # Extract type and points count
                type_str = parts[3].rstrip(',')
                points_str = parts[5].rstrip('):')
                
                contour_type = int(type_str)
                num_points = int(points_str)
                
                current_contour = {
                    'id': contour_id,
                    'type': contour_type,
                    'type_name': 'EXTERNAL' if contour_type == 1 else 'INTERNAL',
                    'points': num_points,
                    'coords': []
                }
            
            # Parse coordinate line: "123.456789 234.567890"
            elif current_contour is not None:
                try:
                    x, y = map(float, line.split())
                    current_contour['coords'].append([x, y])
                except ValueError:
                    # Not a coordinate line, skip
                    pass
    
    # Add last contour if exists
    if current_contour and len(current_contour['coords']) > 0:
        current_contour['coords'] = np.array(current_contour['coords'])
        contours.append(current_contour)
    
    return contours


def visualize_contours(contours, save_path=None):
    """
    Visualize contours using matplotlib.
    
    Args:
        contours: List of contour dictionaries
        save_path: Optional path to save the figure
    """
    fig, axes = plt.subplots(1, 2, figsize=(16, 8))
    
    # Left plot: All contours with different colors
    ax1 = axes[0]
    ax1.set_title('All Contours (Colored by Index)', fontsize=14, fontweight='bold')
    ax1.set_xlabel('X coordinate', fontsize=12)
    ax1.set_ylabel('Y coordinate', fontsize=12)
    ax1.grid(True, alpha=0.3)
    ax1.set_aspect('equal', adjustable='datalim')
    
    # Right plot: Contours colored by type (EXTERNAL/INTERNAL)
    ax2 = axes[1]
    ax2.set_title('Contours by Type', fontsize=14, fontweight='bold')
    ax2.set_xlabel('X coordinate', fontsize=12)
    ax2.set_ylabel('Y coordinate', fontsize=12)
    ax2.grid(True, alpha=0.3)
    ax2.set_aspect('equal', adjustable='datalim')
    
    # Color maps
    colors = plt.cm.tab10(np.linspace(0, 1, len(contours)))
    type_colors = {'EXTERNAL': 'blue', 'INTERNAL': 'red'}
    
    # Plot each contour
    for idx, contour in enumerate(contours):
        coords = contour['coords']
        
        # Left plot: Color by index
        ax1.plot(coords[:, 0], coords[:, 1], 
                color=colors[idx], linewidth=2, 
                label=f"Contour {contour['id']} ({contour['type_name']}, {contour['points']} pts)")
        
        # Mark start point
        ax1.plot(coords[0, 0], coords[0, 1], 'o', 
                color=colors[idx], markersize=8, 
                markeredgecolor='black', markeredgewidth=1)
        
        # Right plot: Color by type
        color = type_colors[contour['type_name']]
        ax2.plot(coords[:, 0], coords[:, 1], 
                color=color, linewidth=2, alpha=0.7,
                label=f"{contour['type_name']} (Contour {contour['id']})")
        
        # Mark start point
        ax2.plot(coords[0, 0], coords[0, 1], 'o', 
                color=color, markersize=8, 
                markeredgecolor='black', markeredgewidth=1)
        
        # Fill polygon for better visualization
        if len(coords) > 2:
            ax2.fill(coords[:, 0], coords[:, 1], color=color, alpha=0.1)
    
    # Add legends
    ax1.legend(loc='best', fontsize=9)
    ax2.legend(loc='best', fontsize=10)
    
    # Invert Y axis to match image coordinates (origin top-left)
    ax1.invert_yaxis()
    ax2.invert_yaxis()
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=150, bbox_inches='tight')
        print(f"Figure saved to: {save_path}")
    
    plt.show()


def print_contour_info(contours):
    """Print detailed information about contours."""
    print("\n" + "="*70)
    print("CONTOUR ANALYSIS")
    print("="*70)
    print(f"Total contours found: {len(contours)}\n")
    
    external_count = sum(1 for c in contours if c['type_name'] == 'EXTERNAL')
    internal_count = sum(1 for c in contours if c['type_name'] == 'INTERNAL')
    
    print(f"EXTERNAL contours: {external_count}")
    print(f"INTERNAL contours: {internal_count}\n")
    
    for contour in contours:
        print(f"Contour {contour['id']}:")
        print(f"  Type: {contour['type_name']} ({contour['type']})")
        print(f"  Points: {contour['points']}")
        
        coords = contour['coords']
        print(f"  Bounds: X=[{coords[:, 0].min():.2f}, {coords[:, 0].max():.2f}], "
              f"Y=[{coords[:, 1].min():.2f}, {coords[:, 1].max():.2f}]")
        
        # Check if closed
        first = coords[0]
        last = coords[-1]
        distance = np.sqrt((first[0] - last[0])**2 + (first[1] - last[1])**2)
        is_closed = distance < 1e-3
        print(f"  Closed: {is_closed} (distance={distance:.6f})")
        print()


def main():
    """Main function to load and visualize contours."""
    import os
    
    # File path
    filename = r"C:\temp\contour_points.txt"
    
    # Check if file exists
    if not os.path.exists(filename):
        print(f"Error: File not found: {filename}")
        print("\nMake sure you've run the application in Debug mode")
        print("and the debug code in FormBoundsOnLoadFile() has executed.")
        return
    
    print(f"Reading contours from: {filename}")
    
    # Parse the file
    contours = parse_contour_file(filename)
    
    if not contours:
        print("No contours found in file!")
        return
    
    # Print information
    print_contour_info(contours)
    
    # Visualize
    save_path = r"C:\temp\contour_visualization.png"
    visualize_contours(contours, save_path)
    
    print("\nVisualization complete!")


if __name__ == "__main__":
    main()
