# sync-builds.ps1 Enhancement: Automatic Filter Organization

## What Was Added

The `sync-builds.ps1` script now automatically maintains the `.vcxproj.filters` file, ensuring that all files are properly organized in Visual Studio's Solution Explorer.

---

## Features

### 1. Automatic Filter Path Detection

The script intelligently determines the correct filter path based on the file location:

```powershell
function Get-FilterPath {
    param([string]$filePath)
    
    # src\geometry\Point.cpp -> Source Files\geometry
    if ($filePath -match '^src\\(\w+)\\') {
        return "Source Files\$($matches[1])"
    }
    
    # include\aperturecore\visibility\TypeLimits.h -> Header Files\visibility
    elseif ($filePath -match '^include\\aperturecore\\(\w+)\\') {
        return "Header Files\$($matches[1])"
    }
    
    # Defaults
    if ($filePath -match '\.cpp$') { return "Source Files" }
    if ($filePath -match '\.h$') { return "Header Files" }
}
```

### 2. Filter Organization Rules

| File Path | Filter Path |
|-----------|-------------|
| `src\geometry\Ellipse.cpp` | `Source Files\geometry` |
| `src\visibility\TypeLimits.cpp` | `Source Files\visibility` |
| `include\aperturecore\geometry\Point.h` | `Header Files\geometry` |
| `include\aperturecore\visibility\ShapeCollection.h` | `Header Files\visibility` |

### 3. Synchronization

The script now:
1. ? Adds missing files to `.vcxproj` 
2. ? Adds missing files to `.vcxproj.filters` with proper organization
3. ? Removes extra files from both
4. ? Creates backups of both files
5. ? Reports all changes

---

## Before vs After

### Before Enhancement

**Problem:** Files added to `.vcxproj` were not added to `.vcxproj.filters`, resulting in:
- Unorganized files in Solution Explorer
- Files appearing at root level
- No filter folder structure

**Visual Studio Solution Explorer:**
```
Solution 'Digit'
??? ApertureCore
    ??? Header Files
    ?   ??? Point.h
    ?   ??? Bounds.h
    ?   ??? TypeLimits.h
    ??? Source Files
    ?   ??? Point.cpp
    ?   ??? Bounds.cpp
    ?   ??? TypeLimits.cpp
    ??? Ellipse.h          ? Wrong! At root level
    ??? Rectangle.h        ? Wrong! At root level
    ??? Polygon.h          ? Wrong! At root level
    ??? Ellipse.cpp        ? Wrong! At root level
    ??? Rectangle.cpp      ? Wrong! At root level
    ??? Polygon.cpp        ? Wrong! At root level
```

### After Enhancement

**Visual Studio Solution Explorer:**
```
Solution 'Digit'
??? ApertureCore
    ??? Header Files
    ?   ??? geometry
    ?   ?   ??? Point.h
    ?   ?   ??? Bounds.h
    ?   ?   ??? Shape.h
    ?   ?   ??? Ellipse.h      ? Properly organized
    ?   ?   ??? Rectangle.h    ? Properly organized
    ?   ?   ??? Polygon.h      ? Properly organized
    ?   ??? visibility
    ?       ??? TypeLimits.h
    ?       ??? ShapeCollection.h
    ?       ??? VisibilityChecker.h
    ??? Source Files
    ?   ??? geometry
    ?   ?   ??? Point.cpp
    ?   ?   ??? Bounds.cpp
    ?   ?   ??? Shape.cpp
    ?   ?   ??? Ellipse.cpp     ? Properly organized
    ?   ?   ??? Rectangle.cpp   ? Properly organized
    ?   ?   ??? Polygon.cpp     ? Properly organized
    ?   ??? visibility
    ?       ??? TypeLimits.cpp
    ?       ??? ShapeCollection.cpp
    ?       ??? VisibilityChecker.cpp
    ??? Documentation
        ??? README.md
        ??? DESIGN.md
        ??? ...
```

---

## Usage

### Run Synchronization

```powershell
cd ApertureCore
.\sync-builds.ps1
```

**Output:**
```
ApertureCore Build System Sync
===============================

Found in CMakeLists.txt:
  Source files: 9
  Header files: 0

Auto-discovered headers: 9

Synchronization Analysis:

  ? All source files in sync
  ? All headers in sync

  ? No changes needed

Done!

Syncing .vcxproj.filters...
  Filters: Added src\geometry\Ellipse.cpp -> Source Files\geometry
  Filters: Added src\geometry\Rectangle.cpp -> Source Files\geometry
  Filters: Added src\geometry\Polygon.cpp -> Source Files\geometry
  Filters: Added include\aperturecore\geometry\Ellipse.h -> Header Files\geometry
  Filters: Added include\aperturecore\geometry\Polygon.h -> Header Files\geometry
  Filters: Added include\aperturecore\geometry\Rectangle.h -> Header Files\geometry

  Filters backup: ApertureCore.vcxproj.filters.backup
  ? Saved ApertureCore.vcxproj.filters

Done!
```

### Dry Run (Preview Changes)

```powershell
.\sync-builds.ps1 -DryRun
```

**Output:**
```
Syncing .vcxproj.filters...
  Would add 3 source files and 3 headers to filters

Done!
```

---

## Technical Details

### Filter File Structure

The `.vcxproj.filters` file maintains:

1. **Filter Definitions** (folder structure):
```xml
<ItemGroup>
  <Filter Include="Source Files\geometry">
    <UniqueIdentifier>{A1234567-...}</UniqueIdentifier>
  </Filter>
  <Filter Include="Header Files\visibility">
    <UniqueIdentifier>{D1234567-...}</UniqueIdentifier>
  </Filter>
</ItemGroup>
```

2. **File Mappings** (files assigned to filters):
```xml
<ItemGroup>
  <ClCompile Include="src\geometry\Ellipse.cpp">
    <Filter>Source Files\geometry</Filter>
  </ClCompile>
  
  <ClInclude Include="include\aperturecore\geometry\Ellipse.h">
    <Filter>Header Files\geometry</Filter>
  </ClInclude>
</ItemGroup>
```

### Script Logic

```powershell
# 1. Parse CMakeLists.txt for source files
$sourceFiles = Parse-CMake

# 2. Auto-discover headers from include/
$allHeaders = Get-ChildItem -Recurse -Filter "*.h"

# 3. Sync .vcxproj
Update-VcxProj $sourceFiles $allHeaders

# 4. Sync .vcxproj.filters (NEW!)
Update-Filters $sourceFiles $allHeaders
```

---

## Benefits

### For Developers

1. **Better Organization** - Files grouped by component (geometry, visibility)
2. **Easier Navigation** - Logical folder structure in Solution Explorer
3. **Consistent Structure** - Matches CMake directory layout
4. **No Manual Work** - Automatic organization on sync

### For Maintainability

1. **Automated** - No manual filter file editing
2. **Consistent** - Always matches CMake source structure
3. **Scalable** - Works for any number of files
4. **Predictable** - Clear rules for filter paths

---

## Pre-Commit Hook Integration

The pre-commit hook now automatically:
1. Syncs `.vcxproj` ?
2. Syncs `.vcxproj.filters` ? **NEW!**

**Result:** Git commits always have both files synchronized!

---

## Edge Cases Handled

### 1. Missing Filters File
```powershell
if (Test-Path $filtersFile) {
    # Sync filters
} else {
    Write-Host "Warning: $filtersFile not found"
}
```

### 2. Missing ItemGroups
```powershell
if ($filtersCompileGroup -and $filtersIncludeGroup) {
    # Process
} else {
    Write-Host "Warning: Could not find ItemGroups in .vcxproj.filters"
}
```

### 3. Unrecognized File Paths
```powershell
$filterPath = Get-FilterPath $file
if ($filterPath) {
    # Add with filter
} # else: skip (no filter assignment)
```

---

## Testing

### Test 1: Add New Geometry File

**Setup:**
```cmake
# CMakeLists.txt
set(APERTURE_SOURCES
    ...
    src/geometry/Circle.cpp  # NEW
)
```

**Run:**
```powershell
.\sync-builds.ps1
```

**Result:**
```
? Added to .vcxproj: src\geometry\Circle.cpp
? Added to filters: src\geometry\Circle.cpp -> Source Files\geometry
```

### Test 2: Add New Visibility File

**Setup:**
```cmake
src/visibility/RayTracer.cpp  # NEW
```

**Result:**
```
? Added to filters: src\visibility\RayTracer.cpp -> Source Files\visibility
```

### Test 3: Remove Obsolete File

**Setup:**
Remove `src/geometry/Old.cpp` from CMakeLists.txt

**Result:**
```
? Removed from .vcxproj: src\geometry\Old.cpp
? Removed from filters: src\geometry\Old.cpp
```

---

## Commit

```bash
git commit -m "feat(sync-builds): add automatic .vcxproj.filters synchronization"

2 files changed, 153 insertions(+)
- sync-builds.ps1 (new filter sync logic)
- ApertureCore.vcxproj.filters (synced 6 files)
```

---

## Summary

The `sync-builds.ps1` script now maintains **both** `.vcxproj` and `.vcxproj.filters` files, ensuring:

1. ? Files are in the Visual Studio project
2. ? Files are organized in proper filter folders
3. ? Folder structure matches CMake layout
4. ? No manual filter file editing needed
5. ? Pre-commit hook keeps everything synchronized

**Result:** Professional, well-organized Visual Studio project structure that matches the CMake source tree! ??
