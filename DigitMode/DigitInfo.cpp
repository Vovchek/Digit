#include "DigitMode/RedCenterDetector.h"
#include "DigitMode/FringeConstructor.h"
#include "DigitMode/ShapeConversionHelpers.h"
#include "DigitInfo.h"
#include "Controls/CApertureCtrls.h"
#include "InterfSolver/INCLUDE/Int_Cons.h"
#include "Utils\mutils.h"
#include "Utils\middle.h"
#include "MGTools\Include\Utils\Utils.h"
#include <math.h>
#include <filesystem>
#include <string>
#include <vector>
#include <windowsx.h>
#include "FringeSegmentAdaptor.h"

#ifndef INTERNAL
#define INTERNAL 0
#endif

#ifndef EXTERNAL
#define EXTERNAL 1
#endif

CDigitInfo::CDigitInfo()
{
	Init();
}

CDigitInfo::~CDigitInfo()
{
	Clear();
}

void CDigitInfo::Init()
{
	SecSegm = -1;
	isInsideScreen = FALSE;
	numStep = 1.;
	MainFringeNumber = -1000.;
	idxMainSection = -1;
	idxDragZapLine = -1;
	idxDragDot = -1;
	idxMainDot = -1;
	HandSetZapLines = FALSE;
	CorrectionSecSegm = 0.3;
	CurrentNumber = 0.;

	Comments = _T("No comments");
	ScaleFactor = 1.;
	Rotation = 0.;
	
	// NEW: Initialize fringe model
	m_bUseFringeModel = TRUE;
	idxDraggedPoint.Clear();
	idxMainPoint.Clear();
}

BOOL CDigitInfo::IsDigiting()
{
	if (Sections.GetSize())
		return TRUE;
	else
		return FALSE;
}

void CDigitInfo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bUseFringeModel) {
		if (!idxMainPoint.IsValid()) return;
		CFringeSegment* pFr = GetFringe(idxMainPoint.iFringe);
		if (!pFr) return;
		int currentIdx = idxMainPoint.iPoint;
		int nextIdx = -1;

		if (nChar == VK_LEFT) {
			nextIdx = currentIdx - 1;
		}
		else if (nChar == VK_RIGHT) {
			nextIdx = currentIdx + 1;
		}
		else if (nChar == VK_UP) {
			nextIdx = currentIdx - 1;
		}
		else if (nChar == VK_DOWN) {
			nextIdx = currentIdx + 1;
		}

		if (nextIdx >= 0 && nextIdx < pFr->GetPointCount()) {
			idxMainPoint.iPoint = nextIdx;
			CurrentNumber = pFr->GetNumber();
		}
		return;
	}

	int idx;
	CDPoint dP;
	if (idxMainDot != -1) {
		dP = Dots[idxMainDot].P;
		if (nChar == VK_LEFT) {
			if (GetNextDotInSection(Dots[idxMainDot].iZapSec, -1, idx, dP)) {
				idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
		else if (nChar == VK_RIGHT) {
			if (GetNextDotInSection(Dots[idxMainDot].iZapSec, 1, idx, dP)) {
				idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
		else if (nChar == VK_UP) {
			if (GetNextDotInFringe(Dots[idxMainDot].Number, -1, idx, dP)) {
			idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
		else if (nChar == VK_DOWN) {
			if (GetNextDotInFringe(Dots[idxMainDot].Number, 1, idx, dP)) {
			 idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
	}
}

void CDigitInfo::Auto()
{
	::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	Clear(FALSE);
	CreateRedCenters();
    if (GetControls() != nullptr) {
        numStep = GetControls()->FringeCenterAs == FC_MINMAX ? .5: 1.;
    }
    m_bUseFringeModel = false;  // TODO: update after transition

    if (!m_bUseFringeModel) {
        SelectFringeStep();     // determines median-average spacing
        SelectMainSection();    // determines section crossing maximum fringes with constrained spacing

		CreateNumLines();       // Assigns fringe number (aka isoline height) to each detected fringe in each section, 
                                // based on proximity to main section and step size
        if (!isInsideScreen) {
			SelectMainFringe(); // find fringe with most matches across sections and set as main fringe (number reference)
			CorrectNumbers();   // Adjust fringe numbers across all sections to be consistent with main fringe and step size
        }

        CreateZAPSections();
        // sort dots by increasing fringe,then by Y downwards
        SortDotsFY();
        SelectMainDot();
        
        SyncFringesToDots(); // bylateral sync fringes <-> dots
        m_bUseFringeModel = true;
    }
    else {

        m_bUseFringeModel = true;

    }
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

void CDigitInfo::CreateRedCenters()
{
    HidenDots.RemoveAll();
    Sections.RemoveAll();

    CImageCtrls* pI = GetImageCtrls();
    DigitMode::CApertureCtrls* pA = GetApertureCtrls();
    CControls* pCtrls = GetControls();
    if (!pI || !pA || !pCtrls)
        return;

    isInsideScreen = pA->GetInternalCount() > 0;

    DigitMode::digitization::DigitizationInput input;

    // RAII guard automatically manages padding state
    CDIBPaddingGuard paddingGuard(pI->m_pDIB);
    input.bitmapData = pI->getBitmapData();

    input.imageWidth = pI->getWidth();
    input.imageHeight = pI->getHeight();
    auto* maskProvider = &pA->GetMaskProvider();
    input.isVisible = [maskProvider](int x, int y) {
        return maskProvider->getMask().IsVisible(x, y);
    };
    input.fringeCenterAs = pCtrls->FringeCenterAs;
    input.contrastThreshold = pCtrls->Eps;

    // TODO: redesign DetectExtrema so it will return vector of Sections -
    // same (or etended) content but MFC-free. Everything can be done in DetectExtrema.
    // Red/Black indicator should persist.
    auto sections = DigitMode::digitization::RedCenterDetector::DetectExtrema(input);

    if (input.imageHeight <= 0 || input.imageWidth <= 0)
        return;

    // =========================================================
    // TODO: eliminate this part after DetectExtrema redesign.
    Sections.SetSize(input.imageHeight);
    std::vector<std::vector<double>> rowCenters(static_cast<size_t>(input.imageHeight));
    rowCenters.reserve(static_cast<size_t>(input.imageHeight));

    for (const auto& s : sections) {
        for (const auto& p : s.points) {
            int y = static_cast<int>(p.position.y + 0.5);
            if (y < 0 || y >= input.imageHeight)
                continue;
            rowCenters[static_cast<size_t>(y)].push_back(p.position.x);
        }
    }

    for (int y = 0; y < input.imageHeight; ++y) {
        Sections[y].Init();
        Sections[y].L.P1.y = Sections[y].L.P2.y = y;
        Sections[y].L.P1.x = sections[y].limits.leftEdge.x;
        Sections[y].L.P2.x = sections[y].limits.rightEdge.x;
        Sections[y].NumLines.RemoveAll();

        const auto& centers = rowCenters[static_cast<size_t>(y)];
        for (double redX : centers) {
            CNumLine numLine;
            numLine.redX = redX;
            Sections[y].NumLines.Add(numLine);
        }
    }
    // TODO: end of code to eliminate
    // =========================================================

    for (const auto& s : sections) {
        for(const auto& p: s.points)
        HidenDots.Add(CDPoint(p.position.x, p.position.y));
    }

    if (0) { // TODO: this is under construction, shold be enabled and moved somewhere else

    auto fringes = DigitMode::digitization::FringeConstructor::ConstructFringes(
        sections, input.imageWidth, input.imageHeight, input.isVisible,
        input.fringeCenterAs, input.fringeStep, 0.3);

        Fringes = DigitMode::digitization::FringeSegmentAdapter::AdaptFringes(fringes);
    }

}
 
void CDigitInfo::Draw(CDC* pDC, int DotSide, CDPoint activeDot, CDPoint cursorPos, bool rubberBand)
{
    CControls* pCtrls = GetControls();
    if (!pCtrls) return;

    // Determine current world transform scale to keep UI pen sizes consistent
    double scale = 1.0;
    XFORM xf; memset(&xf, 0, sizeof(xf));
    if (GetWorldTransform(pDC->GetSafeHdc(), &xf)) {
        scale = xf.eM11 != 0.0f ? xf.eM11 : 1.0;
    }

    // Draw zap sections if enabled
    if (pCtrls->ViewState & V_ZAPSECTIONS) {
        for (int i = 0; i < ZapLines.GetSize(); ++i) {
            if (idxDragZapLine == i || ZapLines[i].Removed) continue;
            ZapLines[i].Draw(pDC);
        }
    }

    // Draw extremums and section previews
    if (pCtrls->ViewState & V_EXTREMUMS) {
        COLORREF col = RGB(255,0,0);
        for (int i = 0; i < HidenDots.GetSize(); ++i) {
            CPoint P((int)HidenDots[i].x, (int)HidenDots[i].y);
            pDC->SetPixelV(P.x, P.y, col);
        }
        for (int i = 0; i < Sections.GetSize(); ++i) {
            Sections[i].Draw(pDC, static_cast<int>(MainFringeNumber));
        }
    }

    // Draw fringe polylines
    if (m_bUseFringeModel) {
        if (pCtrls->ViewState & V_DOTLINES) {
            for (size_t iF = 0; iF < Fringes.size(); ++iF) {
                double num = Fringes[iF].GetNumber();
                COLORREF Color; pCtrls->GetIndexColor(num, Color);
                int w = (int)(1.0 / scale + 0.5); 
                if (w < 1) w = 1;
                
                Fringes[iF].DrawPolyline(pDC, w, Color);
                
            }
        }
    

        // Draw dots and active rubber-band
        if (pCtrls->ViewState & V_DOTS) {
            for (size_t iF = 0; iF < Fringes.size(); ++iF) {
                double num = Fringes[iF].GetNumber();
                COLORREF Color; pCtrls->GetIndexColor(num, Color);
                // Calculate world size to keep dots constant screen size
                double screenDot = DotSide;
                double dotScale = scale;
                int worldDotSide = (int)(screenDot / (dotScale == 0.0 ? 1.0 : dotScale) + 0.5);
                // Prevent dots from disappearing at high zoom: use minimum world size of 2
                if (worldDotSide < 2) worldDotSide = 2;
                Fringes[iF].DrawDots(pDC, worldDotSide, Color);
            }
        }

        if (activeDot != CPoint(-1, -1)) {
            // Use same world-scaling for active dot so it doesn't become huge
            double screenDot = DotSide + 2;
            double dotScale = scale;
            int worldDotSide = (int)(screenDot / (dotScale == 0.0 ? 1.0 : dotScale) + 0.5);
            if (worldDotSide < 2) worldDotSide = 2;
            int half = worldDotSide / 2;
            CBrush brush(RGB(255,64,64)); CBrush* oldBr = pDC->SelectObject(&brush);
            pDC->Ellipse(
                static_cast<int>(activeDot.x-half), 
                static_cast<int>(activeDot.y-half), 
                static_cast<int>(activeDot.x+half), 
                static_cast<int>(activeDot.y+half)
            );
            pDC->SelectObject(oldBr);

            if (cursorPos != CPoint(-1, -1) && rubberBand) {
                // Rubber-band pen: use world width to keep 1px on screen
                //int w = (int)(1.0 / scale + 0.5); if (w < 1) w = 1;
                CPen rubberPen; rubberPen.CreatePen(PS_DOT, 0, RGB(255,128,0));
                CPen* oldPen = pDC->SelectObject(&rubberPen);
                // TODO: use GDI++ for better precision
				CPoint dot = CPoint(static_cast<int>(activeDot.x + 0.5), static_cast<int>(activeDot.y + 0.5));
                pDC->MoveTo(dot);
				CPoint cursor = CPoint(static_cast<int>(cursorPos.x + 0.5), static_cast<int>(cursorPos.y + 0.5));
                pDC->LineTo(cursor);
                pDC->SelectObject(oldPen);
            }
        }
    }
    
    // Draw selection highlighting (glow effect for selected items)
    selectionManager.DrawSelection(pDC, Fringes);
}

void CDigitInfo::Clear(BOOL AllZAPSections/*TRUE*/)
{
	HidenDots.RemoveAll();
	Sections.RemoveAll();
	Dots.clear();
	Fringes.clear();  // NEW: Clear fringes
	
	if (AllZAPSections)
		ZapLines.RemoveAll();
	else if (!HandSetZapLines)
		ZapLines.RemoveAll();

	idxMainSection = -1;
	idxDragZapLine = -1;
	idxDragDot = -1;
	idxMainDot = -1;
	HandSetZapLines = FALSE;

	// NEW: Clear fringe selection
	MainFringeNumber = -1000.;
	idxDraggedPoint.Clear();
	idxMainPoint.Clear();
}


BOOL CDigitInfo::SaveZAP(LPCTSTR fname, int extIdx)
{
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return FALSE;
	
	if (extIdx == 2)
	{
		WriteWinZAPData(fname, IntInfo);
		return true;
	}
	else if (extIdx == 3)
	{
		WriteDosZAPData(fname, IntInfo);
		return true;
	}

	return false;
}

BOOL CDigitInfo::SaveFRN(LPCTSTR fname, bool saveAsWinFringe)
{
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return false;
    if(saveAsWinFringe)
		IntInfo.LoadedFileType = NUMBERING_INTERFEROGRAM_INFO::TYP_WINFRINGE_FRN;
    else
		IntInfo.LoadedFileType = NUMBERING_INTERFEROGRAM_INFO::TYP_FRN;
	return WriteFRNData(fname, IntInfo);
}

#include "digitInfoFringe.hxx" // extracted for testing purposes

#include "digitInfoExt.hxx" // returned back the stuff copilot recklessly removed

