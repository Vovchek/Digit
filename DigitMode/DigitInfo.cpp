#include "DigitMode/RedCenterDetector.h"
#include "DigitMode/FringeConstructor.h"
#include "DigitInfo.h"
#include "Controls/CApertureCtrls.h"
#include "InterfSolver/INCLUDE/Int_Cons.h"
#include "Utils\\mutils.h"
#include "Utils\middle.h"
#include "MGTools\Include\Utils\Utils.h"
#include <math.h>
#include <filesystem>
#include <string>
#include <vector>
#include <windowsx.h>

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
	SelectFringeStep();
	SelectMainSection();

	CreateNumLines();
	if (!isInsideScreen) {
		SelectMainFringe();
		CorrectNumbers();
	}

	CreateZAPSections();
	// sort dots by increasing fringe,then by Y downwards
	SortDotsFY();
	SelectMainDot();
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	m_bUseFringeModel = false; // TODO: update after transition
	SyncFringesToDots();
	m_bUseFringeModel = true;
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
    input.bitmapData = pI->GetBitmapData();
    input.imageWidth = pI->GetWidth();
    input.imageHeight = pI->GetHeight();
    auto* maskProvider = &pA->GetMaskProvider();
    input.isVisible = [maskProvider](int x, int y) {
        return maskProvider->getMask().IsVisible(x, y);
    };
    input.fringeCenterAs = pCtrls->FringeCenterAs;
    input.contrastThreshold = pCtrls->Eps;

    auto redCenters = DigitMode::digitization::RedCenterDetector::DetectExtrema(input);

    if (input.imageHeight <= 0 || input.imageWidth <= 0)
        return;

    Sections.SetSize(input.imageHeight);
    std::vector<std::vector<double>> rowCenters(static_cast<size_t>(input.imageHeight));
    rowCenters.reserve(static_cast<size_t>(input.imageHeight));

    for (const auto& redCenter : redCenters) {
        int y = static_cast<int>(redCenter.position.y + 0.5);
        if (y < 0 || y >= input.imageHeight)
            continue;
        rowCenters[static_cast<size_t>(y)].push_back(redCenter.position.x);
    }

    for (int y = 0; y < input.imageHeight; ++y) {
        Sections[y].Init();
        Sections[y].L.P1.y = Sections[y].L.P2.y = y;

        int left = -1;
        int right = -1;
        for (int x = 0; x < input.imageWidth; ++x) {
            if (input.isVisible && input.isVisible(x, y)) {
                if (left < 0)
                    left = x;
                right = x;
            }
        }
        if (left < 0) {
            left = 0;
            right = 0;
        }
        Sections[y].L.P1.x = left;
        Sections[y].L.P2.x = right;
        Sections[y].NumLines.RemoveAll();

        const auto& centers = rowCenters[static_cast<size_t>(y)];
        for (double redX : centers) {
            CNumLine numLine;
            numLine.redX = redX;
            Sections[y].NumLines.Add(numLine);
        }

        if (pCtrls->FringeCenterAs == FC_MINMAX) {
            Sections[y].Sort();
        }
    }

    for (const auto& redCenter : redCenters) {
        HidenDots.Add(CDPoint(redCenter.position.x, redCenter.position.y));
    }
}
 
void CDigitInfo::Draw(CDC* pDC, int DotSide, CPoint activeDot, CPoint cursorPos, bool rubberBand)
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
    if (pCtrls->ViewState & V_DOTLINES) {
        if (m_bUseFringeModel) {
            for (size_t iF = 0; iF < Fringes.size(); ++iF) {
                double num = Fringes[iF].GetNumber();
                COLORREF Color; pCtrls->GetIndexColor(num, Color);
                int w = (int)(1.0 / scale + 0.5); if (w < 1) w = 1;
                CPen pen(PS_GEOMETRIC, w, Color);
                CPen* oldP = pDC->SelectObject(&pen);
                Fringes[iF].DrawPolyline(pDC, Color);
                pDC->SelectObject(oldP);
            }
        }
    }

    // Draw dots and active rubber-band
    if (pCtrls->ViewState & V_DOTS) {
        if (m_bUseFringeModel) {
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

            if (activeDot != CPoint(-1, -1)) {
                // Use same world-scaling for active dot so it doesn't become huge
                double screenDot = DotSide + 2;
                double dotScale = scale;
                int worldDotSide = (int)(screenDot / (dotScale == 0.0 ? 1.0 : dotScale) + 0.5);
                if (worldDotSide < 2) worldDotSide = 2;
                int half = worldDotSide / 2;
                CBrush brush(RGB(255,64,64)); CBrush* oldBr = pDC->SelectObject(&brush);
                pDC->Ellipse(activeDot.x-half, activeDot.y-half, activeDot.x+half, activeDot.y+half);
                pDC->SelectObject(oldBr);

                if (cursorPos != CPoint(-1, -1) && rubberBand) {
                    // Rubber-band pen: use world width to keep 1px on screen
                    int w = (int)(1.0 / scale + 0.5); if (w < 1) w = 1;
                    CPen rubberPen; rubberPen.CreatePen(PS_DOT, w, RGB(255,128,0));
                    CPen* oldPen = pDC->SelectObject(&rubberPen);
                    pDC->MoveTo(activeDot);
                    pDC->LineTo(cursorPos);
                    pDC->SelectObject(oldPen);
                }
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

BOOL CDigitInfo::SaveFRN(LPCTSTR fname)
{
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return false;
	WriteFRNData(fname, IntInfo);
	return true;
}

#include "digitInfoFringe.hxx" // extracted for testing purposes

#include "digitInfoExt.hxx" // returned back the stuff copilot recklessly removed

