# Urgent needs (000+)
### 001 WinFringe .frn format compatibility issues
* fix ellipses format - they must be normalized. But where to get center and radius for normalization?
* fix bounds format (yet not clear)
### 002 smooth and robust interpolation
### 003 adjust sections spacing to fringes spacing

---
# Bugs (100+)
### 101 <del>Rectangular bound refuses to be stretched with handles</del>
### 102 drag-created rectangular bound is 90-degrees rotated
### 103 <del>Dragging poins and segments aims at the wrong position</del>
### 104 previousHoveredTool points to deleted memory after lose/regain focus
### 105 <del>Some elipsed sizes entail corruption of WavefrontFromContours solvers</del>
    Fixed invalid crossings search methods in WavefrontFromContours that were missing crossings with segments whick verteces are located exactly on scanned line.
### 106 Automatic digitization does not propagate to the bottom if case of rectangular aperture (maybe in some other cases)
### 107 Not reproduced, needs clarification ?Undo do not work on point dragging?
### 108 Segment dragging Execute/Undo is not atomic

# Incomplete (200+)
### 201 Fringes numbers auto-assignment
* test circles
* implement mixed circles/fringes
* inplement saddles

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

## Fringes Navigation mode toolbar (bumeration, etc)
## Add phase calculation
## Add save to mtr
## 