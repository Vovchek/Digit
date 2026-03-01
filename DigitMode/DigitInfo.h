#if !defined(AFX_DIGIT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_DIGIT_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_
//C:\Ilya\Programming\cpp\Numbering\DigitMode\DigitInfo.h
#include "MGTools\StdAfx.h"
#include "Appdef.h"

#include "MGTools\Include\Utils\BaseDataType.h"

#include "DigitMode\SectionInfo.h"
#include "DigitMode\DotInfo.h"
#include "DigitMode\ZapLineInfo.h"
#include "DigitMode\CFringeSegment.h"  // NEW: Fringe-based model
#include "DigitMode/SelectionManager.h"  // Include for SelectionManager

#include "InterfSolver\Tools\ReadWriteData.h"

class CImageCtrls;  // Forward declaration

/**
 * @brief Selection point structure for fringe-based model.
 * 
 * Identifies a specific point within a fringe using (iFringe, iPoint) coordinates.
 * This is used for tracking the currently selected or dragged point in the 
 * segment-primary fringe model.
 * 
 * @see CDigitInfo::idxDraggedPoint
 * @see CDigitInfo::idxMainPoint
 */
struct SelectedPoint {
    int iFringe;  ///< Index into Fringes array (-1 if invalid)
    int iPoint;   ///< Index into Points array within fringe (-1 if invalid)
    
    SelectedPoint() : iFringe(-1), iPoint(-1) {}
    SelectedPoint(int f, int p) : iFringe(f), iPoint(p) {}
    
    /** @brief Check if this selection points to a valid fringe/point pair */
    BOOL IsValid() const { return iFringe >= 0 && iPoint >= 0; }
    
    /** @brief Reset to invalid state */
    void Clear() { iFringe = iPoint = -1; }
    
    /** @brief Equality comparison */
    BOOL operator==(const SelectedPoint& other) const {
        return iFringe == other.iFringe && iPoint == other.iPoint;
    }
};

/**
 * @class CDigitInfo
 * @brief Central data model for interferogram fringe digitization and analysis.
 * 
 * @section Overview
 * CDigitInfo manages all data related to interferogram fringe detection, numbering,
 * and visualization. It supports both automatic and manual digitization workflows.
 * 
 * @section Domain_Concepts Domain Concepts
 * 
 * **Interferogram**: An optical pattern showing interference fringes (light/dark bands)
 * created by coherent light waves. Each fringe represents a constant phase difference.
 * 
 * **Fringe**: A single continuous curve (polyline) connecting points of equal optical
 * path difference. Fringes are numbered sequentially (e.g., 1.0, 1.5, 2.0...).
 * 
 * **Segment**: A continuous polyline representing one fringe. In segment-primary model,
 * fringes are THE primary objects (not containers of segments).
 * 
 * **Red Center (Extremum)**: Local maximum or minimum intensity point along a scan line.
 * These points are candidates for fringe center detection.
 * 
 * **ZAP Section**: Horizontal reference line across the interferogram where fringe centers
 * are detected and numbered. Used for systematic fringe identification.
 * 
 * **Number**: Floating-point identifier assigned to each fringe (e.g., 1.0, 1.5, 2.0).
 * Sequential numbering represents increasing optical path difference.
 * 
 * **Aperture**: Outer boundary defining the region of interest (usually circular/elliptical).
 * 
 * **Obstruction**: Inner boundary defining excluded regions (e.g., central obscuration).
 * 
 * @section Data_Model Data Model
 * 
 * The class uses a **segment-primary** model where:
 * - Each CFringeSegment is a complete polyline (sequence of dots)
 * - Fringes are logical groupings of segments with the same Number
 * - Segments exist independently; fringes are computed on-demand
 * 
 * @section Workflow Workflow
 * 
 * **Automatic Digitization** (Auto()):
 * 1. CreateBufLine() - Build aperture/obstruction masks
 * 2. CreateRedCenters() - Detect fringe extrema in scan lines
 * 3. SelectFringeStep() - Calculate average fringe spacing
 * 4. CreateNumLines() - Connect extrema into continuous fringes
 * 5. CreateZAPSections() - Create vertical scan lines for reference
 * 
 * **Manual Editing**:
 * - Add/remove individual dots
 * - Renumber fringes
 * - Adjust fringe continuity
 * 
 * @section Transition Legacy Transition
 * 
 * The class is transitioning from flat Dots array to segment-based Fringes model:
 * - **Old model**: std::vector<CDotInfo> Dots (flat array of all dots)
 * - **New model**: std::vector<CFringeSegment> Fringes (segment-primary)
 * - **Flag**: m_bUseFringeModel controls which model is active
 * 
 * @author Original: Unknown; Refactored for segment-primary model
 * @date 2024
 */
class CDigitInfo
{
  public:
    // ========================================================================
    // SCAN LINE ANALYSIS DATA
    // ========================================================================
    
    /**
     * @brief Horizontal scan sections containing detected fringe extrema.
     * 
     * **Purpose**: Each CSectionInfo represents one horizontal scan line
     * and contains all detected intensity extrema (red centers) along that line.
     * 
     * **Usage**: Auto() workflow:
     * 1. CreateRedCenters() populates Sections with extrema
     * 2. CreateNumLines() connects extrema vertically to form continuous fringes
     * 
     * **Index**: Sections[i] corresponds to scan line at y = aperture.top + i
     * 
     * @see CreateRedCenters()
     * @see CSectionInfo
     */
    CArray<CSectionInfo> Sections;
    
    /**
     * @brief Array of detected intensity extrema across all scan lines.
     * 
     * **Purpose**: Stores all candidate fringe center points before they are
     * organized into continuous fringes. These are "hidden" because they are
     * not part of the final fringe model—just intermediate detection results.
     * 
     * **Lifecycle**:
     * - Populated by CreateRedCenters() during automatic digitization
     * - Used by CreateNumLines() to build fringe polylines
     * - Cleared by Clear()
     * 
     * **Visualization**: Can be drawn in V_EXTREMUMS view mode for debugging
     * 
     * @see CreateRedCenters()
     * @see Sections
     */
    CArray<CDPoint> HidenDots;
    
    // ========================================================================
    // ZAP SECTION DATA
    // ========================================================================
    
    /**
     * @brief Horizontal reference lines used for fringe identification and numbering.
     * 
     * **Purpose**: ZAP sections are horizontal lines across the interferogram, enabling:
     * - Fringe identification by y-coordinate
     * - Numbering verification
     * - Manual fringe editing
     * - Reference positions for vertical fringe tracking
     * 
     * **Usage**:
     * - CreateZAPSections() generates sections automatically
     * - User can add/delete sections manually
     * - Dots are associated with sections for navigation
     * 
     * @see CZapLineInfo
     * @see CreateZAPSections()
     * @see HandSetZapLines
     */
    CArray<CZapLineInfo> ZapLines;
    
    /**
     * @brief Flag indicating ZAP lines were set manually by user.
     * 
     * **Purpose**: Controls whether Clear() preserves ZAP lines.
     * - TRUE: User created ZAP lines → preserve on Clear(FALSE)
     * - FALSE: Auto-generated ZAP lines → clear on Clear(FALSE)
     * 
     * @see ZapLines
     * @see Clear()
     */
    BOOL HandSetZapLines;
    
    /**
     * @brief Index of currently dragged ZAP line (-1 if none).
     * 
     * **Purpose**: Tracks which ZAP line user is currently dragging with mouse.
     * Used for interactive repositioning of horizontal reference lines.
     * 
     * @see ZapLines
     * @see IsLockedZapSection()
     */
    int idxDragZapLine;
    
    // ========================================================================
    // FRINGE DATA - OLD MODEL (TRANSITIONAL)
    // ========================================================================
    
    /**
     * @brief Flat array of all fringe dots (LEGACY MODEL).
     * 
     * **Purpose**: Original data model where all dots from all fringes are
     * stored in a single flat vector. Each CDotInfo contains:
     * - Position (x, y)
     * - Fringe number
     * - ZAP section index
     * 
     * **Status**: DEPRECATED - being replaced by Fringes (segment-primary model)
     * 
     * **Transition**: When m_bUseFringeModel is FALSE, this model is active.
     * Use SyncDotsToFringes() / SyncFringesToDots() to convert between models.
     * 
     * @deprecated Use Fringes (segment-primary model) instead
     * @see Fringes
     * @see m_bUseFringeModel
     */
    std::vector<CDotInfo> Dots;
    
    /**
     * @brief Index of currently dragged dot in Dots array (LEGACY MODEL).
     * 
     * **Purpose**: Tracks which dot user is dragging in the old flat-array model.
     * 
     * **Status**: DEPRECATED - replaced by idxDraggedPoint in segment-primary model
     * 
     * @deprecated Use idxDraggedPoint instead
     * @see idxDraggedPoint
     */
    int idxDragDot;
    
    /**
     * @brief Index of currently selected "main" dot in Dots array (LEGACY MODEL).
     * 
     * **Purpose**: Tracks the primary selected dot for keyboard navigation
     * and editing operations in the old flat-array model.
     * 
     * **Status**: DEPRECATED - replaced by idxMainPoint in segment-primary model
     * 
     * @deprecated Use idxMainPoint instead
     * @see idxMainPoint
     */
    int idxMainDot;
    
    // ========================================================================
    // FRINGE DATA - NEW MODEL (SEGMENT-PRIMARY)
    // ========================================================================
    
    /**
     * @brief Segment-primary fringe data model (CURRENT MODEL).
     * 
     * **Purpose**: Modern data structure where each fringe is a complete
     * CFringeSegment (polyline) with:
     * - Ordered sequence of points (std::vector<CDPoint>)
     * - Fringe number (e.g., 1.0, 1.5, 2.0)
     * - Segment index
     * 
     * **Segment-Primary Model**:
     * - Each CFringeSegment is THE primary object
     * - Fringes are logical groupings (all segments with same Number)
     * - Segments exist independently
     * - Computed on-demand, not stored as containers
     * 
     * **Usage**:
     * - CreateFringe() / DeleteFringe() manage lifecycle
     * - AddPointToFringe() / RemovePointFromFringe() modify geometry
     * - FindFringesByNumber() queries to find segments by fringe number
     * 
     * **Transition**: When m_bUseFringeModel is TRUE, this model is active.
     * 
     * @see CFringeSegment
     * @see m_bUseFringeModel
     * @see CreateFringe()
     */
    std::vector<CFringeSegment> Fringes;
    
    /**
     * @brief Model selection flag: TRUE = segment-primary, FALSE = flat Dots array.
     * 
     * **Purpose**: Controls which data model is active during transition period.
     * - TRUE: Use Fringes (segment-primary model) ← RECOMMENDED
     * - FALSE: Use Dots (legacy flat array model) ← DEPRECATED
     * 
     * **Default**: TRUE (set in Init())
     * 
     * **Usage**: Check before accessing either Fringes or Dots.
     * 
     * @see Fringes
     * @see Dots
     */
    BOOL m_bUseFringeModel;
    
    /**
     * @brief Currently dragged point in segment-primary model.
     * 
     * **Purpose**: Tracks which point user is dragging in the Fringes model.
     * - iFringe = index into Fringes array
     * - iPoint = index into point array within that fringe
     * 
     * **Replaces**: idxDragDot (from legacy Dots model)
     * 
     * @see SelectedPoint
     * @see idxMainPoint
     * @see FindPointUnderCursor()
     */
    SelectedPoint idxDraggedPoint;
    
    /**
     * @brief Currently selected "main" point in segment-primary model.
     * 
     * **Purpose**: Tracks the primary selected point for:
     * - Keyboard navigation (arrow keys move between points)
     * - Editing operations (renumbering, deletion)
     * - Visual highlighting
     * 
     * **Replaces**: idxMainDot (from legacy Dots model)
     * 
     * **Keyboard Navigation**:
     * - Left/Right: Move along current fringe
     * - Up/Down: Jump to nearest point in adjacent fringe
     * 
     * @see SelectedPoint
     * @see idxDraggedPoint
     * @see OnKeyDown()
     */
    SelectedPoint idxMainPoint;
    
    // ========================================================================
    // SELECTION MANAGEMENT (NAVIGATE MODE)
    // ========================================================================
    
    /**
     * @brief Selection manager for Navigate mode (UX v1.0 segment-primary).
     * 
     * **Purpose**: Manages hierarchical selection state:
     * - Dot selection (individual points)
     * - Edge selection (line between two dots)
     * - Segment selection (entire fringe polyline)
     * - Fringe selection (all segments with same Number)
     * 
     * **Features**:
     * - Box selection with modifiers (Shift=Segment, Alt=Fringe, Ctrl=Add)
     * - Click selection with modifiers
     * - Persistent selection across mode switches
     * - Visual feedback (glow effect)
     * 
     * **Usage**: Accessed by InputHandler in Navigate mode
     * 
     * @see DigitMode::SelectionManager
     * @see DigitMode::InputHandler
     */
    DigitMode::SelectionManager selectionManager;
    
    // ========================================================================
    // FRINGE NUMBERING PROPERTIES
    // ========================================================================
    
    /**
     * @brief Current fringe number for new segments.
     * 
     * **Purpose**: Next fringe created will be assigned this number.
     * Auto-increments by numStep when new fringe is started in Draw mode.
     * 
     * **Example**: If CurrentNumber = 2.5 and numStep = 0.5, next fringe is 3.0
     * 
     * **Usage**:
     * - Draw mode: Auto-increments on new segment
     * - Manual entry: User can set via UI
     * - Keyboard: +/- keys adjust by numStep
     * 
     * @see numStep
     * @see MainFringeNumber
     */
    double CurrentNumber;
    
    /**
     * @brief Increment/decrement step for fringe numbering.
     * 
     * **Purpose**: Defines spacing between sequential fringe numbers.
     * Common values:
     * - 1.0: Integer numbering (1, 2, 3, ...)
     * - 0.5: Half-integer numbering (1.0, 1.5, 2.0, ...)
     * - 0.1: Fine numbering (1.0, 1.1, 1.2, ...)
     * 
     * **Auto-Detection**: SelectFringeStep() calculates this from detected
     * fringe spacing in the interferogram.
     * 
     * @see CurrentNumber
     * @see SelectFringeStep()
     */
    double numStep;
    
    /**
     * @brief Number of the "main" fringe selected by user or auto-detection.
     * 
     * **Purpose**: Identifies a reference fringe for:
     * - Numbering origin (where to start counting)
     * - Visual highlighting
     * - Relative numbering operations
     * 
     * **Default**: -1000.0 (invalid/unset)
     * 
     * **Auto-Selection**: SelectMainFringe() chooses based on:
     * - Longest fringe
     * - Central position
     * - User preference
     * 
     * @see SelectMainFringe()
     * @see CurrentNumber
     */
    double MainFringeNumber;
    
    // ========================================================================
    // SCAN LINE ANALYSIS PARAMETERS
    // ========================================================================
    
    /**
     * @brief Index of the "main" horizontal section used for analysis.
     * 
     * **Purpose**: Identifies a representative scan line used for:
     * - Fringe spacing calculation (SelectFringeStep)
     * - Quality metrics
     * - Reference position
     * 
     * **Default**: -1 (unset)
     * 
     * **Auto-Selection**: SelectMainSection() chooses based on:
     * - Central y-position
     * - Good fringe visibility
     * - Minimal noise
     * 
     * @see Sections
     * @see SelectMainSection()
     */
    int idxMainSection;
    
    /**
     * @brief Average distance between fringes in a section (pixels).
     * 
     * **Purpose**: Stores calculated fringe spacing for a reference section.
     * Used to validate fringe detection and set numStep.
     * 
     * **Calculation**: CalcSectionAveSteps() computes this from detected
     * red centers in a section.
     * 
     * **Default**: -1 (not calculated)
     * 
     * @see CalcSectionAveSteps()
     * @see numStep
     */
    double SecSegm;
    
    /**
     * @brief Correction factor for fringe spacing validation.
     * 
     * **Purpose**: Tolerance for fringe spacing variations.
     * A fringe is valid if its spacing is within ±CorrectionSecSegm of SecSegm.
     * 
     * **Default**: 0.3 (30% tolerance)
     * 
     * **Example**: If SecSegm = 10.0 pixels and CorrectionSecSegm = 0.3,
     * valid spacings are 7.0 to 13.0 pixels.
     * 
     * @see SecSegm
     * @see SelectNumber()
     */
    double CorrectionSecSegm;
    
    // ========================================================================
    // GEOMETRY FLAGS
    // ========================================================================
    
    /**
     * @brief TRUE if aperture has a central obstruction (hole).
     * 
     * **Purpose**: Indicates presence of inner boundary (e.g., central obscuration
     * in a telescope mirror). Affects:
     * - Fringe detection (skip obstructed region)
     * - Numbering strategy (avoid obstruction center)
     * - Rendering (show obstruction boundary)
     * 
     * **Detection**: Set by CreateBufLineObstruction*() if InsBoundType != -1
     * 
     * @see CreateBufLineObstructionSimple()
     * @see buf_line (indices [2] and [3] define obstruction)
     */
    BOOL isInsideScreen;
    
    // ========================================================================
    // METADATA
    // ========================================================================
    
    /**
     * @brief User comments about the interferogram.
     * 
     * **Purpose**: Free-text annotations for:
     * - Measurement conditions
     * - Sample description
     * - Processing notes
     * 
     * **Default**: "No comments"
     * 
     * **Persistence**: Saved/loaded with ZAP and FRN files
     * 
     * @see SetComments()
     * @see GetComments()
     */
    CString Comments;
    
    /**
     * @brief Scale factor for converting pixels to physical units.
     * 
     * **Purpose**: Maps pixel coordinates to real-world dimensions.
     * 
     * **Example**: If ScaleFactor = 0.01 mm/pixel, then 100 pixels = 1 mm
     * 
     * **Default**: 1.0 (no scaling)
     * 
     * **Usage**: Applied during export to physical measurement systems
     * 
     * @see SetScaleFactor()
     */
    double ScaleFactor;
    
    /**
     * @brief Rotation angle of the interferogram (degrees).
     * 
     * **Purpose**: Records rotation applied to align fringes with coordinate axes.
     * Useful for:
     * - Compensating for camera tilt
     * - Aligning with reference orientation
     * 
     * **Default**: 0.0 (no rotation)
     * 
     * @see SetRotation()
     */
    double Rotation;
	  	
  public:
	  CDigitInfo();
	  virtual ~CDigitInfo();
	  
	  /** @brief Initialize all member variables to default state */
	  void Init();

      // ========================================================================
      // MEMORY MANAGEMENT
      // ========================================================================
	  
	  /**
	   * @brief Allocate buf_line array for aperture/obstruction mask.
	   * 
	   * @param ny Number of scan lines (typically aperture height)
	   * @param n Number of boundary values per line (typically 4)
	   * 
	   * **Structure Created**:
	   * - buf_line[ny][4] where:
	   *   - [0] = left aperture edge
	   *   - [1] = right aperture edge
	   *   - [2] = left obstruction edge (-1 if none)
	   *   - [3] = right obstruction edge (-1 if none)
	   * 
	   * **Lifecycle**: Call Delete_buf_line() before re-initializing
	   * 
	   * @see Delete_buf_line()
	   * @see buf_line
	   */
	  void Init_buf_line(int ny, int n);
	  
	  /**
	   * @brief Free buf_line array memory.
	   * 
	   * **Safety**: Safe to call multiple times (sets buf_line to NULL)
	   * 
	   * @see Init_buf_line()
	   */
      void Delete_buf_line();
	  	
	  /**
	   * @brief Check if digitization data exists.
	   * 
	   * @return TRUE if Sections array is non-empty (fringe detection has run)
	   * 
	   * **Usage**: Enables/disables UI controls that require digitized data
	   */
	  BOOL IsDigiting();

      // ========================================================================
      // AUTOMATIC DIGITIZATION WORKFLOW
      // ========================================================================
	  
	  /**
	   * @brief Run complete automatic fringe digitization workflow.
	   * 
	   * **Steps**:
	   * 1. CreateBufLine() - Build aperture/obstruction masks
	   * 2. CreateRedCenters() - Detect fringe extrema in scan lines
	   * 3. SelectFringeStep() - Calculate average fringe spacing
	   * 4. SelectMainSection() - Choose reference scan line
	   * 5. CreateNumLines() - Connect extrema into continuous fringes
	   * 6. SelectMainFringe() - Identify primary fringe (if no obstruction)
	   * 7. CorrectNumbers() - Adjust numbering for consistency
	   * 8. CreateZAPSections() - Generate vertical reference lines
	   * 9. SortDotsFY() - Sort by fringe number, then Y coordinate
	   * 
	   * **Cursor**: Shows wait cursor during processing
	   * 
	   * **Model Transition**: Syncs legacy Dots from new Fringes model
	   * 
	   * @see CreateBufLine()
	   * @see CreateRedCenters()
	   */
	  void Auto();
	  
	  /**
	   * @brief Clear all digitization data.
	   * 
	   * @param AllZAPSections TRUE = clear all ZAP lines, FALSE = preserve manual ZAP lines
	   * 
	   * **Clears**:
	   * - HidenDots (extrema)
	   * - Sections (scan line data)
	   * - Dots (legacy model)
	   * - Fringes (segment-primary model)
	   * - ZapLines (conditionally)
	   * - Selection state
	   * - Main indices (idxMainSection, idxMainDot, etc.)
	   * 
	   * **Preserves**:
	   * - Aperture/obstruction settings
	   * - Numbering parameters (CurrentNumber, numStep)
	   * - Metadata (Comments, ScaleFactor, Rotation)
	   */
	  void Clear(BOOL AllZAPSections=TRUE);

      // ========================================================================
      // MANUAL EDITING OPERATIONS
      // ========================================================================
	  
	  /**
	   * @brief Add a new dot at cursor position.
	   * 
	   * @param P Cursor position (image coordinates)
	   * @param dotSide Hit test tolerance (pixels)
	   * 
	   * **Behavior**:
	   * - If near existing fringe: adds dot to that fringe
	   * - If in empty space: creates new fringe with CurrentNumber
	   * 
	   * **Model**: Works with both Dots and Fringes models
	   */
	  void AddDot(CPoint P, int dotSide);
	  
	  /**
	   * @brief Remove dot at cursor position.
	   * 
	   * @param P Cursor position
	   * @param dotSide Hit test tolerance
	   * 
	   * **Behavior**:
	   * - Finds nearest dot within dotSide radius
	   * - Removes dot from fringe
	   * - If fringe becomes empty, removes fringe
	   */
	  void RemoveDot(CPoint P, int dotSide);
	  
	  /**
	   * @brief Remove entire fringe at cursor position.
	   * 
	   * @param P Cursor position
	   * @param dotSide Hit test tolerance
	   * 
	   * **Behavior**:
	   * - Finds fringe under cursor
	   * - Removes all dots in that fringe
	   * - In segment-primary model: removes all segments with same Number
	   */
	  void RemoveFringe(CPoint P, int dotSide);
	  
	  /**
	   * @brief Remove all dots in a ZAP section.
	   * 
	   * @param iSec Index into ZapLines array
	   * 
	   * **Purpose**: Clears detected fringes along a vertical scan line
	   */
	  void RemoveDotZAPSection(int iSec);
	  
	  /**
	   * @brief Add a new ZAP section at specified y-coordinate.
	   * 
	   * @param iy Y-coordinate (image coordinates)
	   * 
	   * **Purpose**: Creates vertical scan line for manual fringe editing
	   */
      void AddZapSection(int iy);
      
      /**
       * @brief Delete ZAP section at y-coordinate.
       * 
       * @param iy Y-coordinate
       */
      void DeleteZapSection(int iy);
      
      /**
       * @brief Change fringe number at cursor position.
       * 
       * @param P Cursor position
       * @param dotSide Hit test tolerance
       * 
       * **Behavior**:
       * - Prompts user for new number (via UI)
       * - Renumbers entire fringe to new value
       * - In segment-primary model: changes Number property of all matching segments
       */
  	  void RenumFringe(CPoint P, int dotSide);
  	  
  	  /**
  	   * @brief Change number of single dot.
  	   * 
  	   * @param P Cursor position
  	   * @param dotSide Hit test tolerance
  	   * 
  	   * **Warning**: Can break fringe continuity! Use with caution.
  	   */
  	  void RenumDot(CPoint P, int dotSide);
  	  
  	  /**
  	   * @brief Decrement CurrentNumber by numStep.
  	   * 
  	   * **Effect**: CurrentNumber -= numStep
  	   * 
  	   * **Usage**: Keyboard shortcut '-' or UI button
  	   */
	  void NumberMinus();
	  
	  /**
	   * @brief Increment CurrentNumber by numStep.
  	   * 
  	   * **Effect**: CurrentNumber += numStep
  	   * 
  	   * **Usage**: Keyboard shortcut '+' or UI button
  	   */
	  void NumberPlus();

      // ========================================================================
      // APERTURE/OBSTRUCTION MASK GENERATION
      // ========================================================================

      /**
       * @brief Build complete aperture/obstruction mask (calls both Aperture and Obstruction).
       * 
       * **Workflow**:
       * 1. CreateBufLineAperture() - Outer boundary
       * 2. CreateBufLineOnstruction() - Inner boundary
       * 
       * @see buf_line
       */
      void CreateBufLine();
      
      /**
       * @brief Build aperture (outer boundary) mask.
       * 
       * **Routing**: Calls Simple or Complex variant based on boundary type
       * 
       * @see CreateBufLineApertureSimple()
       * @see CreateBufLineApertureComplex()
       */
      void CreateBufLineAperture();
      
      /**
       * @brief Build obstruction (inner boundary) mask.
       * 
       * **Routing**: Calls Simple or Complex variant based on boundary type
       * 
       * @see CreateBufLineObstructionSimple()
       * @see CreateBufLineObstructionComplex()
       */
      void CreateBufLineOnstruction();
      
      /**
       * @brief Build aperture mask for polygonal boundary.
       * 
       * **Algorithm**: Scan-line polygon fill
       * 
       * **Complexity**: O(n * m) where n = height, m = polygon vertices
       */
	  void CreateBufLineApertureComplex();
	  
	  /**
	   * @brief Build aperture mask for simple shapes (circle, ellipse, rectangle).
	   * 
	   * **Algorithm**:
	   * - Circle/Ellipse: Analytical boundary calculation
	   * - Rectangle: Direct assignment
	   * 
	   * **Performance**: O(n) where n = height
	   */
	  void CreateBufLineApertureSimple();
	  
	  /**
	   * @brief Build obstruction mask for polygonal inner boundary.
	   * 
	   * @see CreateBufLineApertureComplex()
	   */
	  void CreateBufLineObstructionComplex();
	  
	  /**
	   * @brief Build obstruction mask for simple shapes.
	   * 
	   * @see CreateBufLineApertureSimple()
	   */
	  void CreateBufLineObstructionSimple();

      // ========================================================================
      // FRINGE DETECTION AND ANALYSIS
      // ========================================================================
	  
	  /**
	   * @brief Detect fringe extrema (red centers) in all scan lines.
	   * 
	   * **Algorithm**:
	   * 1. For each horizontal scan line within aperture:
	   *    a. Extract pixel intensities
	   *    b. Apply background subtraction (fon_del)
	   *    c. Find local maxima/minima
	   *    d. Store as red centers
	   * 2. Populate HidenDots with all detected extrema
	   * 3. Populate Sections with per-line extrema
	   * 
	   * **Controls**:
	   * - FC_MAX: Detect bright fringes (maxima)
	   * - FC_MIN: Detect dark fringes (minima)
	   * - FC_MINMAX: Detect both
	   * 
	   * **Dependencies**: Requires buf_line to be initialized
	   * 
	   * @see HidenDots
	   * @see Sections
	   * @see CreateBufLine()
	   * 
	   * @todo Remove UI dependency (uses CImageCtrls, CDC)
	   */
	  void CreateRedCenters();
	  
	  /**
	   * @brief Render all digitization data to device context.
	   * 
	   * @param pDC Device context (with world transform applied)
	   * @param DotSide Dot radius in screen pixels
	   * @param activeDot Active dot position (highlighted) or (-1,-1) for none
	   * @param cursorPos Current cursor position (for rubber band) or (-1,-1)
	   * @param rubberBand TRUE to draw rubber band from activeDot to cursorPos
	   * 
	   * **Rendering Layers** (controlled by ViewState flags):
	   * - V_ZAPSECTIONS: Vertical ZAP lines
	   * - V_EXTREMUMS: Red centers (HidenDots)
	   * - V_DOTLINES: Fringe polylines
	   * - V_DOTS: Fringe dots (filled circles)
	   * - Selection: Glow effect for selected items
	   * 
	   * **World Transform**: Assumes DC has world transform set by caller
	   * for proper scaling/translation.
	   * 
	   * @see selectionManager
	   * 
	   * @todo Remove UI dependency (uses CDC)
	   */
	  void Draw(CDC* pDC, int DotSide, CDPoint activeDot, CDPoint cursorPos, bool rubberBand = false);

      // ========================================================================
      // FRINGE SPACING ANALYSIS
      // ========================================================================
	  /**
	   * @brief Calculate average fringe spacing for all sections.
	   * 
	   * **Purpose**: Determines SecSegm for each CSectionInfo
	   * 
	   * @see SecSegm
	   */
	  void CalcSectionAveSteps();
	  
	  /**
	   * @brief Determine optimal fringe numbering step (numStep).
	   * 
	   * **Algorithm**:
	   * 1. Analyze average fringe spacing across sections
	   * 2. Choose step that best matches spacing (1.0, 0.5, 0.1, etc.)
	   * 
	   * **Output**: Sets numStep
	   * 
	   * @see numStep
	   * @see CalcSectionAveSteps()
	   */
	  void SelectFringeStep();
	  
	  /**
	   * @brief Select reference scan line for fringe analysis.
	   * 
	   * **Criteria**:
	   * - Central y-position
	   * - Good fringe visibility (many detected extrema)
	   * - Consistent spacing
	   * 
	   * **Output**: Sets idxMainSection
	   * 
	   * @see idxMainSection
	   */
	  void SelectMainSection();

      // ========================================================================
      // FRINGE CONSTRUCTION
      // ========================================================================

      /**
       * @brief Connect detected extrema into continuous fringe polylines.
       * 
       * **Algorithm**:
       * 1. Start from first scan line
       * 2. For each extremum, find nearest extremum in next line
       * 3. Connect if distance is within tolerance
       * 4. Assign fringe numbers based on spacing
       * 
       * **Output**: Populates Fringes with continuous polylines
       * 
       * **Dependencies**: Requires HidenDots and Sections to be populated
       * 
       * @see CreateRedCenters()
       * @see Fringes
       */
	  void CreateNumLines();
	  
	  /**
	   * @brief Assign numbers to fringes in a scan line.
	   * 
	   * **Strategy**: Uses spacing and continuity to determine numbering
	   * 
	   * @see CreateNumLines()
	   */
	  void NumberingLine();
	  
	  /**
	   * @brief Select main reference fringe for numbering origin.
	   * 
	   * **Criteria**:
	   * - Longest fringe
	   * - Central position (if no obstruction)
	   * - Good continuity
	   * 
	   * **Output**: Sets MainFringeNumber
	   * 
	   * @see MainFringeNumber
	   */
	  void SelectMainFringe();
	  
	  /**
	   * @brief Correct fringe numbering inconsistencies.
	   * 
	   * **Purpose**: Fixes:
	   * - Numbering gaps
	   * - Duplicate numbers
	   * - Reversed sequences
	   * 
	   * **Algorithm**: Propagates numbering from MainFringeNumber outward
	   * 
	   * @see MainFringeNumber
	   */
  	  void CorrectNumbers();
  	  
  	  /**
  	   * @brief Find appropriate fringe number for a detected extremum.
  	   * 
  	   * @param iSec Section index
  	   * @param Sign Direction to search (-1 = left, +1 = right)
  	   * @param redX X-coordinate of extremum
  	   * @param[out] idxS Output section index
  	   * @param[out] idxL Output line index within section
  	   * @return TRUE if valid number found
  	   * 
  	   * **Validation**: Uses CorrectionSecSegm tolerance for spacing
  	   */
      bool SelectNumber(int iSec, int Sign, double redX, int& idxS, int& idxL);

	  // ========================================================================
      // ZAP SECTION MANAGEMENT
      // ========================================================================

      /**
       * @brief Check if ZAP sections exist.
       * 
       * @return TRUE if ZapLines array is non-empty
       */
	  bool IsSections();
	  
	  /**
	   * @brief Generate horizontal ZAP sections for fringe reference.
	   * 
	   * **Algorithm**:
	   * 1. Divide aperture height into regular intervals
	   * 2. Create horizontal line at each interval
	   * 3. Associate detected fringes with nearest ZAP section
	   * 
	   * **Output**: Populates ZapLines array
	   * 
	   * @see ZapLines
	   */
	  void CreateZAPSections();
	  
	  /**
	   * @brief Create ZAP sections when loading from ZAP file.
	   * 
	   * **Difference from CreateZAPSections()**:
	   * - Preserves existing ZAP positions from file
	   * - Does not auto-generate
	   * 
	   * @see LoadZAP()
	   */
	  void CreateZAPSectionsOnLoadZAPFile();
	  
	  /**
	   * @brief Sort ZapLines array by y-coordinate.
	   * 
	   * **Purpose**: Ensures top-to-bottom ordering for navigation
	   */
 	  void SortZapLines();
 	  
 	  /**
 	   * @brief Sort Dots array by fringe number, then Y coordinate.
 	   * 
 	   * **Order**: Primary = fringe number (ascending), Secondary = Y (descending)
 	   * 
 	   * **Purpose**: Enables keyboard navigation (Up/Down between fringes)
 	   */
      void SortDotsFY();
      
      /**
       * @brief Associate dots with a ZAP section.
       * 
       * @param iZAPSec Index into ZapLines array
       * 
       * **Purpose**: Links detected fringes to horizontal reference line
       */
      void PutDotsOnZAPSections(int iZAPSec);

      // ========================================================================
      // DOT SELECTION AND NAVIGATION
      // ========================================================================

      /**
       * @brief Select main dot near cursor position.
       * 
       * @param P Cursor position
       * @param dotSide Hit test tolerance
       * 
       * **Effect**: Sets idxMainDot or idxMainPoint (depending on model)
       * 
       * @see idxMainDot
       * @see idxMainPoint
       */
	  void SelectMainDot(CPoint P, int dotSide);
	  
	  /**
	   * @brief Select main dot by section and fringe number.
	   * 
	   * @param iSec ZAP section index (-1 for current)
	   * @param Number Fringe number (INT_MIN for current)
	   * 
	   * **Purpose**: Programmatic selection without cursor position
	   */
      void SelectMainDot(int iSec=-1, double Number=INT_MIN);
	  	
	  /**
	   * @brief Check if any dots exist.
	   * 
	   * @return TRUE if Dots array is non-empty (legacy model) or Fringes exist
	   */
      bool IsDots();
      
      /**
       * @brief Check if a dot is currently locked for dragging.
       * 
       * @return TRUE if idxDragDot != -1 or idxDraggedPoint.IsValid()
       * 
       * @see LockDot()
       */
      bool IsLockedDot();
      
      /**
       * @brief Lock/unlock dot for dragging.
       * 
       * @param P Cursor position
       * @param dotSide Hit test tolerance
       * @param Enable TRUE to lock, FALSE to unlock
       * @return TRUE if dot found and locked/unlocked
       * 
       * **Usage**: Called on mouse down/up for drag operations
       * 
       * @see IsLockedDot()
       * @see SetLockedDotPos()
       */
      bool LockDot(CPoint P, int dotSide, BOOL Enable);
      
      /**
       * @brief Get dot by section and fringe number.
       * 
       * @param iZapSec ZAP section index
       * @param Number Fringe number
       * @param[out] idx Output dot index
       * @param[out] dP Output dot position
       * @return TRUE if dot found
       */
      bool GetDot(int iZapSec, double Number, int& idx, CDPoint& dP);
      
      /**
       * @brief Update position of locked (dragged) dot.
       * 
       * @param P New position
       * 
       * **Constraints**: May snap to ZAP section if in section edit mode
       * 
       * @see LockDot()
       */
      void SetLockedDotPos(CPoint P);
      
      /**
       * @brief Get position of locked dot.
       * 
       * @param[out] P1 Output position
       * 
       * @see LockDot()
       */
      void GetLockedDotPos(CPoint& P1);
      
      /**
       * @brief Get list of all unique fringe numbers.
       * 
       * @param[out] Numbers Output list of fringe numbers
       * @return TRUE if any fringes exist
       * 
       * **Purpose**: Populate UI dropdowns, statistics
       */
      bool GetDotNumbers(CList<double, double>& Numbers);
      
      /**
       * @brief Get first dot in a ZAP section.
       * 
       * @param iSec Section index
       * @param[out] idx Output dot index
       * @param[out] dP Output dot position
       * @return TRUE if section has dots
       */
	  bool GetFirstDotInSection(int iSec, int& idx, CDPoint& dP);
	  
	  /**
	   * @brief Get next dot in a ZAP section (for keyboard navigation).
	   * 
	   * @param iSec Section index
	   * @param direct Direction (-1 = up, +1 = down)
	   * @param[inout] idx Current dot index (input), next dot index (output)
	   * @param[out] dP Output dot position
	   * @return TRUE if next dot found
	   * 
	   * **Usage**: Left/Right arrow key navigation
	   */
	  bool GetNextDotInSection(int iSec, int direct, int& idx, CDPoint& dP);
	  
	  /**
	   * @brief Check if cursor is near any dot.
	   * 
	   * @param P Cursor position
	   * @param dotSide Hit test radius
	   * @param[out] idx Output dot index if found
	   * @return TRUE if dot within radius
	   * 
	   * **Purpose**: Cursor feedback, click detection
	   */
	  bool IsDotUnderCursor(CPoint P, int dotSide, int& idx);

      // ========================================================================
      // ZAP SECTION SELECTION
      // ========================================================================

      /**
       * @brief Check if ZAP sections exist.
       * 
       * @return TRUE if ZapLines array is non-empty
       */
      bool IsZapSections();
      
      /**
       * @brief Check if a ZAP section is locked for dragging.
       * 
       * @return TRUE if idxDragZapLine != -1
       */
      bool IsLockedZapSection();
      
      /**
       * @brief Check if cursor is near a ZAP section.
       * 
       * @param P Cursor position
       * @param[out] idx Output ZAP section index
       * @return TRUE if near a section
       */
      bool IsZapSectionUnderCursor(CPoint P, int& idx);
      
      /**
       * @brief Lock/unlock ZAP section for dragging.
       * 
       * @param P Cursor position
       * @param Enable TRUE to lock, FALSE to unlock
       * @return TRUE if section found and locked/unlocked
       */
	  bool LockZapSection(CPoint P, BOOL Enable);
	  
	  /**
	   * @brief Update Y position of locked ZAP section.
	   * 
	   * @param iy New Y coordinate
	   * 
	   * **Usage**: Called during vertical drag of horizontal ZAP line
	   */
      void SetLockedZapSectionYPos(int iy);
      
      /**
       * @brief Shift dots left in a ZAP section (decrease fringe number).
       * 
       * @param P Cursor position
       * @param dotSide Hit test tolerance
       * 
       * **Effect**: Finds dot near P, decrements its fringe number by numStep
       */
      void SectionLeft(CPoint P, int dotSide);
      
      /**
       * @brief Shift dots right in a ZAP section (increase fringe number).
       * 
       * @param P Cursor position
       * @param dotSide Hit test tolerance
       * 
       * **Effect**: Finds dot near P, increments its fringe number by numStep
       */
      void SectionRight(CPoint P, int dotSide);
      
      /**
       * @brief Get endpoints of locked ZAP section.
       * 
       * @param[out] P1 Left endpoint
       * @param[out] P2 Right endpoint
       * 
       * **Note**: ZAP sections are horizontal lines, so P1.y == P2.y
       */
      void GetLockedZapSectionXYPos(CPoint& P1, CPoint& P2);

      // ========================================================================
      // FRINGE QUERIES
      // ========================================================================

      /**
       * @brief Get all dot indices for a fringe number (legacy model).
       * 
       * @param Number Fringe number
       * @param[out] idxDots Output array of dot indices
       * @return TRUE if fringe found
       * 
       * @deprecated Use FindFringesByNumber() for segment-primary model
       */
      bool GetFringeDots(double Number, CUIntArray& idxDots);
      
      /**
       * @brief Get all dot positions for a fringe number (legacy model).
       * 
       * @param Number Fringe number
       * @param[out] adP Output array of dot positions
       * @return TRUE if fringe found
       * 
       * @deprecated Use CFringeSegment::GetPoints() for segment-primary model
       */
      bool GetFringeDots(double Number, CArray<CDPoint>& adP);
      
      /**
       * @brief Get first dot in a fringe (legacy model).
       * 
       * @param Number Fringe number
       * @param[out] idx Output dot index
       * @param[out] dP Output dot position
       * @return TRUE if fringe found
       */
	  bool GetFirstDotInFringe(double Number, int& idx, CDPoint& dP);
	  
	  /**
	   * @brief Get next dot in a fringe (keyboard navigation, legacy model).
	   * 
	   * @param Number Fringe number
	   * @param direct Direction (-1 = previous, +1 = next)
	   * @param[inout] idx Current dot index (input), next dot index (output)
	   * @param[out] dP Output dot position
	   * @return TRUE if next dot found
	   * 
	   * **Usage**: Up/Down arrow key navigation between fringes
	   */
      bool GetNextDotInFringe(double Number, int direct, int& idx, CDPoint& dP);
      
      /**
       * @brief Find ZAP section nearest to cursor.
       * 
       * @param P Cursor position
       * @param[out] idx Output section index
       * @return TRUE if section found
       */
      bool GetNearestZapSection(CPoint P, int& idx);
      
      /**
       * @brief Find nearest X coordinate in a ZAP section.
       * 
       * @param P Cursor position
       * @param[out] x Output X coordinate
       * @return TRUE if found
       * 
       * **Purpose**: Snap cursor to ZAP section line
       */
	  bool GetNearestXInSection(CPoint P, double& x);

      // ========================================================================
      // KEYBOARD NAVIGATION
      // ========================================================================

      /**
       * @brief Handle keyboard navigation in digitization data.
       * 
       * @param nChar Virtual key code (VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN)
       * @param nRepCnt Repeat count (usually 1)
       * @param nFlags Key flags
       * 
       * **Navigation** (segment-primary model):
       * - Left/Right: Move along current fringe
       * - Up/Down: Jump to nearest point in adjacent fringe
       * 
       * **Navigation** (legacy model):
       * - Left/Right: Move within ZAP section
       * - Up/Down: Move between fringes
       * 
       * **Effect**: Updates idxMainPoint or idxMainDot
       * 
       * @todo Remove UI dependency (uses UINT types from Windows)
       */
	  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

      // ========================================================================
      // METADATA ACCESSORS
      // ========================================================================

      /** @brief Set user comments */
	  void SetComments(LPCTSTR _Comments){Comments = _Comments;}
	  
	  /** @brief Set scale factor (pixels to physical units) */
      void SetScaleFactor(double _ScaleFactor){ScaleFactor = _ScaleFactor;}
      
      /** @brief Set rotation angle (degrees) */
      void SetRotation(double _Rotation){Rotation = _Rotation;}
      
      /** @brief Get user comments */
	  void GetComments(CString& _Comments){_Comments = Comments;}
	  
	  /** @brief Get scale factor */
      void GetScaleFactor(double& _ScaleFactor){_ScaleFactor = ScaleFactor;}
      
      /** @brief Get rotation angle */
      void GetRotation(double& _Rotation){_Rotation = Rotation;}

      // ========================================================================
      // FILE I/O
      // ========================================================================

      /**
       * @brief Collect digitization data into export structure.
       * 
       * @param[out] IntInfo Output interferogram data structure
       * @return TRUE if data is valid for export
       * 
       * **Purpose**: Prepares data for saving to ZAP or FRN format
       * 
       * @see NUMBERING_INTERFEROGRAM_INFO
       */
      BOOL CollectNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO &IntInfo);
      
      /**
       * @brief Validate imported interferogram data.
       * 
       * @param IntInfo Input data structure
       * @return TRUE if data is valid
       * 
       * **Checks**:
       * - Valid fringe numbers
       * - Consistent point counts
       * - Within aperture bounds
       */
      BOOL ExamineNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO &IntInfo);
      
      /**
       * @brief Load digitization data from file (auto-detect format).
       * 
       * @param fname File path
       * @return TRUE if loaded successfully
       * 
       * **Formats**: Auto-detects ZAP or FRN based on extension
       * 
       * @todo Remove file dependency (use stream or callback)
       */
	  BOOL Load(LPCTSTR fname);
	  
	  /**
	   * @brief Save digitization data to file.
	   * 
	   * @param fname File path
	   * @param extIdx Format index (2 = WinZAP, 3 = DosZAP, other = FRN)
	   * @return TRUE if saved successfully
	   * 
	   * @todo Remove file dependency
	   */
	  BOOL Save(LPCTSTR fname, int extIdx);
	  
	  /**
	   * @brief Load from ZAP format file.
	   * 
	   * @param fname File path
	   * @return TRUE if loaded successfully
	   * 
	   * **ZAP Format**: Legacy format with fringe numbers and positions
	   * 
	   * @see SaveZAP()
	   * @todo Remove file dependency
	   */
	  BOOL LoadZAP(LPCTSTR fname);
	  
	  /**
	   * @brief Load from FRN format file.
	   * 
	   * @param fname File path
	   * @return TRUE if loaded successfully
	   * 
	   * **FRN Format**: Modern format with full polyline data
	   * 
	   * @see SaveFRN()
	   * @todo Remove file dependency
	   */
	  BOOL LoadFRN(LPCTSTR fname);
	  
	  /**
	   * @brief Save to ZAP format file.
	   * 
	   * @param fname File path
	   * @param extIdx Sub-format (2 = Windows, 3 = DOS)
	   * @return TRUE if saved successfully
	   * 
	   * @see LoadZAP()
	   * @todo Remove file dependency
	   */
	  BOOL SaveZAP(LPCTSTR fname, int extIdx);
	  
	  /**
	   * @brief Save to FRN format file.
	   * 
	   * @param fname File path
	   * @return TRUE if saved successfully
	   * 
	   * @see LoadFRN()
	   * @todo Remove file dependency
	   */
	  BOOL SaveFRN(LPCTSTR fname);

  // ===== NEW: Fringe-based interface =====
  public:
	  // ========================================================================
	  // SEGMENT-PRIMARY FRINGE MODEL (UX v1.0)
	  // ========================================================================
	  /**
       * @brief Create new fringe segment and return its index.
       * 
       * @param number Fringe number to assign (e.g., 1.0, 1.5, 2.0)
       * @param segment Segment index within fringe (-1 for auto-assign)
       * @return Index into Fringes array
       * 
       * **Segment-Primary Model**:
       * - Each CFringeSegment is a complete polyline
       * - Fringes are logical groupings (all segments with same Number)
       * - Segments exist independently
       * 
       * **Usage**:
       * ```cpp
       * int iFringe = CreateFringe(1.5, -1);
       * AddPointToFringe(iFringe, CDPoint(100, 200));
       * ```
       * 
       * @see CFringeSegment
       * @see Fringes
       */
	  int CreateFringe(double number, int segment = -1);
	  	
	  /**
	   * @brief Delete fringe segment by index.
	   * 
	   * @param iFringe Index into Fringes array
	   * 
	   * **Warning**: Does NOT delete other segments with same Number.
	   * To delete entire fringe (all segments), use FindFringesByNumber()
	   * and delete all matching indices.
	   * 
	   * **Effect**:
	   * - Removes segment from Fringes array
	   * - Invalidates indices > iFringe
	   * - Updates selection if deleted segment was selected
	   * 
	   * @see FindFringesByNumber()
	   */
	  void DeleteFringe(int iFringe);
	  	
	  /**
	   * @brief Get mutable fringe segment by index.
	   * 
	   * @param i Index into Fringes array
	   * @return Pointer to CFringeSegment or nullptr if invalid
	   * 
	   * **Bounds Check**: Returns nullptr for out-of-range indices
	   * 
	   * @see GetFringe() const
	   */
	  CFringeSegment* GetFringe(int i);
	  
	  /**
	   * @brief Get const fringe segment by index.
	   * 
	   * @param i Index into Fringes array
	   * @return Const pointer to CFringeSegment or nullptr if invalid
	   */
	  const CFringeSegment* GetFringe(int i) const;
	  	
	  /**
	   * @brief Find all segment indices with a given fringe number.
	   * 
	   * @param number Fringe number to search for
	   * @param[out] indices Output vector of matching segment indices
	   * 
	   * **Purpose**: Finds all segments belonging to a logical fringe.
	   * In segment-primary model, a "fringe" is the set of all segments
	   * with the same Number.
	   * 
	   * **Example**:
	   * ```cpp
	   * std::vector<int> indices;
	   * FindFringesByNumber(2.5, indices);
	   * // indices now contains all segments with Number=2.5
	   * ```
	   * 
	   * @see Fringes
	   */
	  void FindFringesByNumber(double number, std::vector<int>& indices) const;
	  	
	  /**
	   * @brief Append point to end of fringe segment.
	   * 
	   * @param iFringe Index into Fringes array
	   * @param p Point to add
	   * 
	   * **Effect**: Calls CFringeSegment::AddPoint()
	   * 
	   * **Usage**: Building fringes during automatic digitization or drawing
	   * 
	   * @see CFringeSegment::AddPoint()
	   */
	  void AddPointToFringe(int iFringe, CDPoint p);
	  	
	  /**
	   * @brief Insert point at specific position in fringe segment.
	   * 
	   * @param iFringe Index into Fringes array
	   * @param iPoint Index where to insert (0 = prepend)
	   * @param p Point to insert
	   * 
	   * **Effect**: Shifts existing points at iPoint and beyond by 1
	   * 
	   * **Usage**: Manual editing, edge insertion
	   * 
	   * @see CFringeSegment::InsertPoint()
	   */
	  void InsertPointInFringe(int iFringe, int iPoint, CDPoint p);
	  	
	  /**
	   * @brief Remove point from fringe segment.
	   * 
	   * @param iFringe Index into Fringes array
	   * @param iPoint Point index within segment
	   * 
	   * **Effect**: If segment becomes empty, consider deleting segment
	   * 
	   * @see CFringeSegment::RemovePoint()
	   * @see DeleteFringe()
	   */
	  void RemovePointFromFringe(int iFringe, int iPoint);
	  	
	  /**
	   * @brief Move point to new position within fringe segment.
	   * 
	   * @param iFringe Index into Fringes array
	   * @param iPoint Point index within segment
	   * @param newP New position
	   * 
	   * **Effect**: Updates point coordinates in-place
	   * 
	   * **Usage**: Drag operations, manual adjustments
	   * 
	   * @see CFringeSegment::SetPoint()
	   */
	  void MovePointInFringe(int iFringe, int iPoint, CDPoint newP);
	  	
	  /**
	   * @brief Find point under cursor with hit testing.
	   * 
	   * @param P Cursor position (image coordinates)
	   * @param tolerance Hit test radius (pixels)
	   * @param[out] outFringe Output fringe segment index
	   * @param[out] outPoint Output point index within segment
	   * @return TRUE if point found within tolerance
	   * 
	   * **Algorithm**: Finds nearest point within tolerance radius
	   * 
	   * **Usage**: Click detection, hover feedback
	   * 
	   * @see FindFringeUnderCursor()
	   */
	  BOOL FindPointUnderCursor(CPoint P, int tolerance, int& outFringe, int& outPoint);
	  	
	  /**
	   * @brief Find fringe under cursor.
	   * 
	   * @param P Cursor position (image coordinates)
	   * @param tolerance Hit test radius (pixels)
	   * @param[out] outFringe Output fringe segment index
	   * @return TRUE if fringe found within tolerance
	   * 
	   * **Algorithm**: Finds nearest fringe polyline within tolerance radius
	   * 
	   * **Usage**: Click detection, hover feedback
	   */
	  BOOL FindFringeUnderCursor(CPoint P, int tolerance, int& outFringe);
	  	
	  /**
	   * @brief Renumber all fringes with oldNumber to newNumber.
	   * 
	   * @param oldNumber Old fringe number
	   * @param newNumber New fringe number
	   * 
	   * **Effect**: Updates Number property of all matching segments
	   * 
	   * **Usage**: Batch renumbering, fixing gaps
	   * 
	   * @see DeleteFringesByNumber()
	   */
	  void RenumberFringes(double oldNumber, double newNumber);
	  	
	  /**
	   * @brief Delete all fringes with given number.
	   * 
	   * @param number Fringe number to delete
	   * 
	   * **Effect**: Calls DeleteFringe() on all matching segments
	   */
	  void DeleteFringesByNumber(double number);
	  	
	  /**
	   * @brief Merge two fringes (must have same number).
	   * 
	   * @param iFringe1 Index of first fringe to merge
	   * @param iFringe2 Index of second fringe to merge
	   * @return TRUE if merged successfully
	   * 
	   * **Effect**: Combines points of both fringes into iFringe1, deletes iFringe2
	   * 
	   * **Constraints**:
	   * - Both fringes must have the same fringe number
	   * - Cannot merge if either fringe is empty
	   * 
	   * **Usage**:
	   * - Automatic: Consolidating split fringes
	   * - Manual: Combining user-defined fringes
	   * 
	   * @see CreateFringe()
	   * @see DeleteFringe()
	   */
	  BOOL MergeFringes(int iFringe1, int iFringe2);
	  	
	  // ===== Conversion utilities (Transition only) =====
	  	
	  /// Convert Dots array to Fringes
	  void ConvertDotsToFringes();
	  	
	  /// Convert Fringes to Dots array
	  void ConvertFringesToDots();
	  	
	  /// Synchronize Fringes to Dots (maintains both models)
	  void SyncFringesToDots();

  protected:
	  void ProcessSectionPropagation(int sectionIndex, int direction, 
		  CArray<CNumLine>& refNumLines, int& maxSize, 
		  double& minN, double& maxN, double& leftX, double& rightX);
private:
    // Helper method to resolve relative image path to absolute path
    static std::string ResolveImagePath(const std::string& dataFilePath, const std::string& imageFileName);
    
    // Helper method to create a fake gray image when actual image is missing
    static BOOL CreateFakeGrayImage(CImageCtrls* pImageCtrls, int width, int height);
};
#endif // !defined(AFX_DIGIT_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
