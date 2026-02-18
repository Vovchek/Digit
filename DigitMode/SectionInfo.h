#if !defined(AFX_SECTION_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_SECTION_INFO_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_

#include "MGTools\StdAfx.h"
#include "Appdef.h"
#include "MGTools\Include\Utils\BaseDataType.h"

/// <summary>
/// Represents a numbered fringe line in the interferogram.
/// Each fringe is assigned a unique number and spatial position for tracking
/// across multiple scan line sections during the numbering propagation process.
/// </summary>
/// <remarks>
/// CNumLine stores both the logical number assigned to a fringe and its physical
/// position in the image. The segment (segmL) defines a tolerance window around
/// the fringe center (redX) used for matching fringes across adjacent sections.
/// 
/// Used by: CSectionInfo to maintain a collection of numbered fringes per section.
/// Created by: CDigitInfo::CreateNumLines() during the numbering propagation phase.
/// </remarks>
class CNumLine : public CObject
{
  public:	
	/// <summary>Logical fringe number assigned during propagation (e.g., 0, 1, 2...)</summary>
	double Number;
	
	/// <summary>Horizontal line segment defining the tolerance window for fringe matching</summary>
	/// <remarks>
	/// P1.x = redX - SecSegm12 (left boundary)
	/// P2.x = redX + SecSegm12 (right boundary)
	/// Used by SelectNumber() to determine if a fringe position matches this numbered fringe.
	/// </remarks>
	CDLine segmL;
	
	/// <summary>Flag indicating whether this fringe has been assigned a number</summary>
	/// <remarks>
	/// TRUE: Fringe has been successfully numbered and is valid for matching
	/// FALSE: Fringe slot is empty or reserved (used during collision resolution)
	/// </remarks>
	BOOL Included;
	
	/// <summary>X-coordinate of the detected fringe center (reduced/processed position)</summary>
	/// <remarks>
	/// "redX" = reduced X position after image processing
	/// This is the primary spatial coordinate used for fringe matching and propagation
	/// </remarks>
	double redX;

  public:
	/// <summary>Default constructor - initializes fringe line to default values</summary>
	CNumLine();
	
	/// <summary>Destructor - cleans up fringe line resources</summary>
	~CNumLine();
	
	/// <summary>Initializes or resets all fringe line members to default values</summary>
	void Init();
	
	/// <summary>Copy constructor - creates a deep copy of another fringe line</summary>
	/// <param name="rhs">Source fringe line to copy from</param>
    CNumLine(const CNumLine& rhs){
      { operator=(rhs);}
     }

	/// <summary>Assignment operator - copies all members from another fringe line</summary>
	/// <param name="rhs">Source fringe line to copy from</param>
	/// <returns>Reference to this fringe line for chaining</returns>
    CNumLine& operator=(const CNumLine& rhs);
};

/// <summary>
/// Represents a horizontal section (scan line) in the interferogram containing detected fringes.
/// Each section corresponds to a single Y-coordinate and stores all fringes detected along that line.
/// </summary>
/// <remarks>
/// CSectionInfo is a fundamental data structure in the fringe numbering algorithm.
/// The main section (selected by SelectMainSection) serves as the starting point,
/// and numbering propagates upward and downward to adjacent sections by matching
/// fringe positions across neighboring scan lines.
/// 
/// Workflow:
/// 1. Form() - Detects fringes along a scan line and populates NumLines array
/// 2. CalcAveStep() - Computes average spacing between fringes
/// 3. CreateNumLines() - Assigns numbers to fringes via propagation
/// 4. Draw() - Renders the section and its numbered fringes
/// 
/// Used by: CDigitInfo::Sections array to store all scan line sections
/// </remarks>
class CSectionInfo : public CObject
{
   public: 
	 /// <summary>Horizontal line defining the Y-position of this section in the image</summary>
	 /// <remarks>
	 /// Typically: P1.y = P2.y = section Y coordinate
	 /// P1.x and P2.x define the horizontal extent of the section
	 /// </remarks>
     CDLine L;
	 
	 /// <summary>Array of all numbered fringe lines detected in this section</summary>
	 /// <remarks>
	 /// Populated by Form() with detected fringes
	 /// Numbered by CreateNumLines() propagation algorithm
	 /// Sorted by Sort() in ascending X-position order
	 /// Each element represents one detected fringe with its assigned number
	 /// </remarks>
	 CArray<CNumLine> NumLines;
	 
	 /// <summary>Average spacing (step) between consecutive fringes in this section</summary>
	 /// <remarks>
	 /// Computed by CalcAveStep() as mean of distances between adjacent fringes
	 /// Value of -1 indicates not calculated or invalid
	 /// Used to determine section-specific tolerance (SecSegm12) during fringe matching
	 /// Helps account for varying fringe density across different parts of the interferogram
	 /// </remarks>
	 double aveStep;
	 
	 /// <summary>Color used when rendering this section's line and fringes</summary>
	 /// <remarks>
	 /// Main section typically uses a distinct color (e.g., red) for visual identification
	 /// Other sections may use default or alternate colors
	 /// </remarks>
	 COLORREF Color;
	 
	 /// <summary>Flag indicating if this is the main section (starting point for numbering)</summary>
	 /// <remarks>
	 /// TRUE: This section is the main section selected by SelectMainSection()
	 /// FALSE: This is an auxiliary section that receives propagated numbering
	 /// Only one section should have MainLine = TRUE at any time
	 /// ⚠️ NOT USED
	 /// </remarks>
	 BOOL MainLine;
	 
	 /// <summary>Flag controlling visibility of red dots marking fringe centers</summary>
	 /// <remarks>
	 /// TRUE: Draw red dots at detected fringe positions (redX) for debugging/visualization
	 /// FALSE: Only draw fringe numbers without position markers
	 /// Useful for verifying fringe detection accuracy
	 /// ⚠️ NOT USED
	 /// </remarks>
	 BOOL VisibleRedDots;

   public: 
	/// <summary>Default constructor - initializes section to default values</summary>
    CSectionInfo();
    
	/// <summary>Destructor - cleans up section resources and fringe arrays</summary>
    ~CSectionInfo();
	
	/// <summary>Initializes or resets all section members to default values</summary>
	void Init();
	
	/// <summary>Renders this section and its numbered fringes to a device context</summary>
	/// <param name="pDC">Device context for drawing</param>
	/// <param name="MainFringeNumber">Number of the main fringe for highlighting or reference</param>
	void Draw(CDC* pDC, int MainFringeNumber);
	
	/// <summary>Sorts the NumLines array by ascending X-position (redX)</summary>
	/// <remarks>
	/// Called after fringe detection to ensure left-to-right ordering
	/// Simplifies fringe matching and propagation algorithms
	/// </remarks>
	void Sort();
	
	/// <summary>Calculates the average spacing between consecutive fringes in this section</summary>
	/// <remarks>
	/// Sets aveStep to the mean distance between adjacent fringes
	/// Used to determine section-specific matching tolerance during propagation
	/// Should be called after Sort() to ensure valid fringe ordering
	/// </remarks>
    void CalcAveStep();
	
	/// <summary>Copy constructor - creates a deep copy of another section</summary>
	/// <param name="rhs">Source section to copy from</param>
    CSectionInfo(const CSectionInfo& rhs){
      { operator=(rhs);}
     }

	/// <summary>Assignment operator - copies all members from another section</summary>
	/// <param name="rhs">Source section to copy from</param>
	/// <returns>Reference to this section for chaining</returns>
    CSectionInfo& operator=(const CSectionInfo& rhs);
};

#endif // !defined(AFX_SECTION_INFO_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
