# Interferogram Processing Application
## Master Specification for Copilot (MFC, Single-Measurement)

---

## 0. Purpose of This Document

This document is a **single authoritative master specification** intended to be provided to **GitHub Copilot inside Visual Studio** as contextual guidance for implementing a desktop application for interferogram processing.

Goals:
- Ensure **architectural correctness**
- Prevent Copilot from mixing UI, math, and pipeline responsibilities
- Preserve **metrological validity** (units, calibration, reproducibility)
- Enable incremental implementation using legacy MFC code

This document intentionally prioritizes **clarity and rigidity** over brevity.

---

## 1. Application Overview

The application processes optical interferograms to reconstruct, analyze, and simulate wavefronts.

Supported capabilities:
- Amplitude-based digitization (manual / assisted)
- Phase reconstruction (PSI, spatial carrier)
- Phase unwrapping
- Wavefront reconstruction
- Calibration wavefront subtraction
- Polynomial / modal analysis (Zernike, Legendre, etc.)
- Diffraction analysis (PSF, MTF, Strehl)
- Interferogram synthesis (inverse modeling)
- Wavefront calculator (engineering operations)
- Rich visualization (2D, 3D, sections, charts)

---

## 2. High-Level Architectural Decisions

### 2.1 Application Type

- **SDI (Single Document Interface)**
- Exactly **one active Measurement** at any time
- No true MDI

Rationale:
- Linear processing pipeline
- Deterministic history / undo
- Reduced architectural complexity
- Consistent with professional interferometry software

### 2.2 Core Principle

> **One Measurement = One Physical Measurement**

All results, parameters, and history are strictly tied to a single Measurement object.

---

## 3. Measurement Model

```cpp
class Measurement
{
public:
    // Raw input
    RawImages images;

    // Geometry & masking
    Aperture aperture;                 // defined early (S0)
    ReferenceMarkers markers;           // scaling & distortion
    GeometricCalibration geometry;

    // Pipeline results
    PhaseData phase;
    WavefrontData wavefront;
    WavefrontData calibratedWavefront;

    PolynomialFitResult polyFit;
    DiffractionResult diffraction;
    SynthesisResult synthesis;

    // Calibration
    CalibrationWavefront calibrationWF;

    // History
    History history;
};
```

Rules:
- `Measurement` owns all data
- Pipeline nodes only **mutate Measurement**
- Views never store persistent results

---

## 4. Units and Coordinate Systems (CRITICAL)

### 4.1 Allowed Units

- Wavelength: **nanometers (nm)**
- Wavefront values: **nanometers (nm)** or **micrometers (μm)**
- Physical dimensions: **millimeters (mm)**

❌ Meters are NOT used anywhere

### 4.2 Coordinate Systems

| Name | Usage |
|----|------|
| Pixel space | Raw images |
| Normalized pupil | Wavefront storage |
| Physical space | Diffraction scaling |

WavefrontData is always stored in **normalized pupil coordinates**.

---

## 5. Processing Pipeline (Canonical)

```
S0   Raw Images
S0a  Aperture & Visibility Masking
S0b  Reference Markers
S0c  Geometric Calibration
S1   Amplitude Digitization
S2   Phase Reconstruction
S3   Phase Unwrapping
S4   Wavefront Reconstruction
S4b  Wavefront Calibration (subtraction)
S5   Polynomial / Modal Analysis
S6   Diffraction Analysis
S7   Interferogram Synthesis
```

Rules:
- Pipeline is strictly ordered
- Each stage invalidates downstream stages
- No stage performs UI rendering

---

## 6. Aperture & Visibility (S0a)

- Aperture is defined **before any processing**
- Used for:
  - masking invalid pixels
  - defining pupil
  - all downstream stages

Aperture types:
- Circular
- Annular
- Rectangular
- Polygonal
- Free mask

Aperture and visibility logic already exists and is **MFC-free**.

---

## 7. Reference Markers & Geometry (S0b, S0c)

Reference markers are visually detectable points with known physical positions.

Used for:
- scaling normalized pupil → mm
- correcting distortion
- aligning synthesized data

Markers may be:
- manually selected
- automatically detected
- loaded from file

---

## 8. Wavefront Reconstruction (S4)

Wavefront definition:
- Scalar field W(x,y)
- Stored in normalized pupil
- Units: nm or μm

Operations:
- phase → wavefront conversion
- piston removal
- tilt removal (optional)
- defocus removal (optional)

WavefrontData contains:
- values grid
- aperture mask
- RMS / PV

---

## 9. Calibration Wavefront (S4b)

Calibration Wavefront represents **systematic interferometer error**.

Rules:
- Loaded from file or measurement
- Stored as WavefrontData snapshot
- Applied as:

```
W_corrected = W_measured − W_calibration
```

Calibration:
- Is NOT Zernike removal
- DOES modify data
- Happens before polynomial fit

---

## 10. Polynomial / Modal Analysis (S5)

Purpose:
- Analyze wavefront composition
- Extract coefficients

Supported bases:
- Zernike (circular / annular)
- Legendre (rectangular)
- User-defined

Important rules:
- Polynomial fit does NOT modify wavefront
- Mode removal is visualization-only

---

## 11. Diffraction Analysis (S6)

Models:
- Fraunhofer (primary)
- Fresnel (optional)

Outputs:
- PSF
- MTF
- OTF
- Strehl ratio

Uses:
- calibrated wavefront
- aperture
- physical scaling (λ, f)

---

## 12. Interferogram Synthesis (S7)

Inverse modeling:
- static interferograms
- phase-shifting (PSI)
- spatial carrier

Used for:
- algorithm verification
- test data generation
- training

---

## 13. Wavefront Calculator

Engineering tool (not pipeline stage):
- add / subtract wavefronts
- scale
- add known Zernike modes
- generate reference wavefronts

Results may be:
- previewed
- committed to Measurement
- saved as calibration

---

## 14. UI Architecture (MFC)

### 14.1 Core Rules

- Views NEVER compute
- Pipeline nodes NEVER draw
- Parameters are edited in dedicated panels

### 14.2 Main Layout

- Menu + toolbar
- Pipeline tree (left)
- Active view (center)
- Dockable parameter panels
- Status bar with key metrics

### 14.3 Document/View Mapping

- One `CDocument` = one `Measurement`
- Views observe Measurement state

---

## 15. History & Undo

- Every parameter change is recorded
- Algorithm execution creates snapshot
- Undo restores exact prior state

No implicit recomputation.

---

## 16. Repository Structure (Recommended)

```
/core
  /math
  /optics
  /wavefront
  /polynomial
  /diffraction

/geometry
  /aperture
  /markers
  /distortion

/pipeline
  nodes
  pipeline.h

/ui_mfc
  views
  panes
  legacy_wrappers

/io
  import
  export

/tests
```

---

## 17. Copilot Hard Rules (Summary)

- Single-measurement application
- One Measurement per document
- Units: nm, μm, mm only
- Calibration WF is subtracted before polynomial fit
- Zernike removal is visualization-only
- No math in views
- No drawing in pipeline nodes

---

## 18. End of Specification

This document is the **single source of truth** for implementation.

All generated code must conform to this specification.

