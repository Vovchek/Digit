# Urgent needs (000+)
### 001 WinFringe .frn format compatibility issues
<del>* fix ellipses format - they must be normalized. But where to get center and radius for normalization?</del>
<del>* fix bounds format (yet not clear)</del>
     * rotate y FisCan before saving to .mtr (find circumcircle: get center, expand bounds to radius, 
       rotate by -FiScan update bounds, shrink to bounds)
### 002 smooth and robust interpolation
### 003 <del>adjust sections spacing to fringes spacing</del>
### 004 <del>use external screens in place of apertures in WinFringe compatibility mode</del>
### 005 <del>set numStep to 0.5 when both dark and bright extremums are detected</del>
### 006 Implement image rotation - image must be saved in rotated position, not just displayed as rotated
### 007 Inplement copy/cut/paste buffer for fringes
### 008 Implement Gauss blur
### 009 Allow to change step for numbering
### 010 Scale surface by ScaleFactor when calculating wavefront - WinFringe ignores ScaleFactor & FiScan/Rotation (maybe)

---
# Bugs (100+)
### 101 <del>Rectangular bound refuses to be stretched with handles</del>
### 102 drag-created rectangular bound is 90-degrees rotated
### 103 <del>Dragging poins and segments aims at the wrong position</del>
### 104 previousHoveredTool points to deleted memory after lose/regain focus
### 105 <del>Some ellipses sizes entail corruption of WavefrontFromContours solvers</del>   
### 106 After connecting segments active segment switches to other segment
### 107 <del>Auto numbering of rings is incorrect</del>

   Done: Fixed invalid crossings search methods in WavefrontFromContours that were missing crossings with segments whick verteces are located exactly on scanned line.

### 106 <del>Automatic digitization does not propagate to the bottom if case of rectangular aperture (maybe in some other cases)</del>
### 107 Not reproduced, needs clarification ?Undo do not work on point dragging?
### 108 Segment dragging Execute/Undo is not atomic
### 109 <del>ellipses and bounds duplicate (not pruned properly on read?)</del>

# Incomplete (200+)
### 201 Fringes numbers auto-assignment
* <del>test circles</del>
* <del>implement mixed circles/fringes</del>
* redesign UX for fringes numbering in select mode: if selection exists, renumber fringes only within selection,
  first fringe is anchor, direction is defined by step sign
* implement saddles

### 202 Fringes auto-tracing   
* more refactoring
* clean dependences
* strategy pattern
* encapsulate to command for undo/redo

### LSM/SVD ellipse fitting

---
# Need refiment (300+)

## <u>Fringes editor</u>
### 300 Fix ringes thickness in pixels
### 301 Less glow/thiknes for selected fringes
### 302 Review cursors
### 303 Fringes navigation with `<Tab>` in Navigation mode

## <u>Bounds editor</u>
### <del>310 ? Make handles permanent in Selection mode</del>
### 311 ? Hide old bound's position while dragging
### 312 Make obstruction(semi-)transparent

---
# Epic feats (500+)

## 501 Fringes Navigation mode toolbar (numeration, etc)
## 502 Add phase calculation
## 503 Implement fiducials for ref.sys transformation and distortion correction
