#if !defined(AFX_BOUND_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_BOUND_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_
//C:\Ilya\Programming\cpp\Numbering\Controls\BoundCtrls.h
#include "MGTools\stdafx.h"
#include "Appdef.h"
#include "MGTools\Include\Utils\BaseDataType.h"

#include "InterfSolver\Tools\CalcLimits.h"
#include "ApertureCore\include\ApertureCore\Visibility\IDataProviders.h"

class CBoundCtrls : public IBoundsData  // ← Implement interface
{
public:
// Legacy members for current bound being edited (before commit) - can be used for both external and internal bounds, as only one is edited at a time
   CRect CurBound;
   CPoint CustomDot;
   CArray<CPoint, CPoint> CustomDots; // bound being currently edited via dots
   CArray<CPoint, CPoint> CurPlg;   // currently edited polygon points for BOUND_POLYGON
   CArrayXYEllipse ArrEll;          // Array of commited ellipses (for BOUND_ELLIPSE)
   CArrayXYRect ArrRect;            // Array of commited rectangles (for BOUND_RECT)
   CArrayXYPolygon ArrPlg;          // Array of commited polygons (for BOUND_POLYGON)
   CArrayXYPolygon ArrContour;      // Resulting contour after combining all bounds for isPupil test
   int ExtBoundType;
   int InsBoundType;
   CArray<int, int> LastAddedBoundType; // keep track of the type (shape in fact) of each added bound
   int NPntNax; // max (magic) number of points for polygon contour approximation

   // new aperture
   // aperture::ShapeCollection Bounds; // collection of all shapes (external and internal) for isPupil test, updated on each commit of a bound

public:
    CBoundCtrls();
    ~CBoundCtrls();
	void Init();
    CBoundCtrls(const CBoundCtrls& rhs){
      { operator=(rhs);}
     }

    CBoundCtrls& operator=(const CBoundCtrls& rhs);

	void AddCustomDot(CPoint P);
    bool RemoveCustomDot(CPoint P);
    bool CustomDotInFocus(CPoint P);

	BOOL IsExtBound(); // returns TRUE if there is at least one external bound (ellipse, rectangle, or polygon)
	BOOL IsInsBound(); // returns TRUE if there is at least one internal bound (ellipse, rectangle, or polygon)
	BOOL IsCurArea(); // returns TRUE if bound being edited is consistent (ready to commit)
	BOOL GetPartsOfContours(int Type, CArrayXYEllipse& _ArrEll, CArrayXYRect& _ArrRect, CArrayXYPolygon& _ArrPlg); // getter for the parts of contours of given type (external or internal)
	
    // IBoundsData interface implementation
	virtual int GetExtBoundType() const { return ExtBoundType; } // Return the shape of external bound (ellipse, rectangle, polygon, or none)
	virtual int GetInsBoundType() const { return InsBoundType; } // Return the shape of internal bound (ellipse, rectangle, polygon, or none)
	virtual BOOL GetExtRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints); // gets external bounds interception cropped to image size
	virtual BOOL GetInsRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints); // gets internal bounds combined cropped to image size
    
    // Legacy overload with idx parameter (keep for existing code)
	BOOL GetInsRealBound(int Type, int xDIB, int yDIB, int& idx, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints); // searches & gets first internal bound starting with idx, cropes to image size; 
	// midifies idx to that of a found bound, sets Bound to its extents, fills PlgPoints with a contour, return true if bound was found
    
	BOOL GetExtCorBound(int Type, int xDIB, int yDIB, CRect& Bound, BOOL XCor, BOOL YCor); // gets external bounds, with optional correction of coordinates to be within image
	BOOL GetInsCorBound(int Type, int xDIB, int yDIB, CRect& Bound, BOOL XCor, BOOL YCor); // gets internal bounds, with optional correction of coordinates to be within image
    void RemoveExtBound(); // actually clears everything
	void RemoveInsBound(); // actually clears everything
	void RemoveAllBound(); // clears everything twice {RemoveExtBound();RemoveInsBound();}
	void RemoveLastBound(); // removes last added bound of any type (external or internal) based on LastAddedBoundType
	bool AddBound(int _Type, int idxExtIns); // calls SetBound(), then clears CurBound and CurPlg
	bool SetBound(int _Type, int idxExtIns); // commits current bound being edited (CurBound and CurPlg), adds to array and recalc contour
    bool SetCurBound(int _Type); // calculates all bounds' structs but does not store (like stage vs commit) 
    void RemoveCurBound(); // cancels edit state
    void FormBoundsOnLoadFile(); // fits its name
	
protected:	
   CRect ExtBoundRect; // aperture extents
   CRect InsBoundRect; // obstruction extents
};

#endif // !defined(AFX_BOUND_CONTROLS_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
