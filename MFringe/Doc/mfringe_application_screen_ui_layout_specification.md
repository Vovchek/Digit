# MFringe
## Application Screen & UI Layout Specification (for Copilot)

---

## 0. Purpose

This document describes the **screen layout, UI structure, and interaction model** of the **MFringe** desktop application.

It is intended to be used as **direct context for GitHub Copilot** when generating MFC UI code.

Scope:
- Main window layout
- Docking panels
- Views and responsibilities
- UI-to-pipeline interaction rules

This document does **not** describe algorithms or math (see MASTER_SPEC.md).

---

## 1. Application Type

- Desktop application
- Windows
- **MFC SDI** (Single Document Interface)
- One active **Measurement** at any time

Application name: **MFringe**

---

## 2. Global UI Principles (Hard Rules)

These rules MUST be respected in all UI code:

1. Views **never perform calculations**
2. Pipeline nodes **never draw UI**
3. All computations are triggered explicitly
4. UI reflects pipeline state, not vice versa
5. Any parameter change invalidates downstream stages
6. Units shown to user: **nm, μm, mm** only

---

## 3. Main Window Layout (Canonical)

```
┌──────────────────────────────────────────────────────────────┐
│ Menu Bar                                                     │
├──────────────────────────────────────────────────────────────┤
│ Toolbar                                                      │
├──────────────────────────────────────────────────────────────┤
│ ┌───────────────┐ ┌───────────────────────────────────────┐ │
│ │ Pipeline Tree │ │ Active View                            │ │
│ │               │ │                                       │ │
│ │ S0 Raw        │ │   (Image / WF / PSF / Charts)         │ │
│ │ S0a Aperture  │ │                                       │ │
│ │ S0b Markers   │ │                                       │ │
│ │ S0c Geometry  │ │                                       │ │
│ │ S1 Digitize   │ │                                       │ │
│ │ S2 Phase      │ │                                       │ │
│ │ S3 Unwrap     │ │                                       │ │
│ │ S4 Wavefront  │ │                                       │ │
│ │ S4b Calib WF  │ │                                       │ │
│ │ S5 Polynomial │ │                                       │ │
│ │ S6 Diffraction│ │                                       │ │
│ │ S7 Synthesis  │ │                                       │ │
│ └───────────────┘ └───────────────────────────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│ Status Bar: RMS | PV | Strehl | λ                           │
└──────────────────────────────────────────────────────────────┘
```

---

## 4. Menu Structure

### 4.1 File
- New Measurement
- Open Images
- Save Measurement
- Export Results
- Exit

### 4.2 View
- Pipeline Tree
- Parameters Panel
- Status Bar

### 4.3 Processing
- Run Pipeline
- Run From Selected Stage
- Recompute Current Stage

### 4.4 Tools
- Wavefront Calculator
- Calibration Manager

### 4.5 Help
- About MFringe

---

## 5. Pipeline Tree Panel

### Purpose

- Visual representation of pipeline state
- Navigation between stages
- Status indication

### Structure

```
S0  Raw Images          [✓]
S0a Aperture           [✓]
S0b Markers            [!]
S0c Geometry           [ ]
S1  Digitization       [ ]
S2  Phase              [ ]
...
```

### Status Indicators
- ✓ valid
- ! warning
- ✗ error
- ☐ not executed

Pipeline Tree:
- is **read-only**
- never modifies data

---

## 6. Active View Area

### Description

The central area displays the view associated with the selected pipeline stage.

Only **one active view** at a time.

### Possible Views

| Stage | View Type |
|------|----------|
| S0 | ImageView |
| S0a | ApertureView |
| S1 | DigitizationView |
| S2 | PhaseView |
| S4 | WavefrontView |
| S5 | PolynomialAnalysisView |
| S6 | DiffractionView |
| S7 | SynthesisView |

---

## 7. Parameters Panel (Dockable)

### Purpose

- All user-editable parameters
- No rendering
- No calculations

### Examples

- Aperture parameters
- Phase reconstruction settings
- Polynomial fit order
- Diffraction wavelength

Parameter changes:
- invalidate downstream stages
- require explicit recompute

---

## 8. Status Bar

Displays **key metrics of the currently valid stage**:

- RMS (nm)
- PV (nm)
- Strehl
- Wavelength (nm)

Status bar never triggers calculations.

---

## 9. Calibration UI

### Calibration Manager Panel

```
Calibration Wavefront
---------------------
[x] Enable calibration
Source: Zygo_ref_2024.txt
RMS: 18.2 nm
[Load] [Clear]
```

Rules:
- Calibration modifies wavefront data
- Enabling/disabling invalidates S5+

---

## 10. Polynomial Analysis UI

### Elements

- Coefficient table
- RMS-per-mode bar chart
- Mode selection (checkboxes)

Mode removal:
- visualization only
- does not modify Measurement

---

## 11. Diffraction UI

### Tabs
- PSF
- MTF
- OTF

Controls:
- λ (nm)
- focal length (mm)
- scaling mode

---

## 12. Synthesis UI

### Controls

- Mode (Static / PSI / Carrier)
- Phase steps
- Carrier frequency
- Noise level

Output:
- Generated interferograms

---

## 13. Error & Warning Presentation

- Errors are shown in:
  - Pipeline Tree
  - Status bar
- Warnings do not block navigation

---

## 14. Naming Conventions (UI Classes)

- CMainFrame
- CMeasurementDoc
- CPipelineTreePane
- CParameterPane
- CWavefrontView
- CDiffractionView

---

## 15. Copilot UI Rules Summary

- Follow this document strictly
- Do not invent additional panels
- Do not merge views
- Do not compute in views
- Always reference Measurement state

---

## End of Document

This document defines the **canonical UI layout** of MFringe.

All UI code must conform to it.

