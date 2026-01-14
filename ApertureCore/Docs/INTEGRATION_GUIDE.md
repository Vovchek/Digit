# Integrating ApertureCore into Digit Solution

## Overview

Three methods to add ApertureCore to the Digit Visual Studio solution:

---

## ? Method 1: Visual Studio GUI (Easiest)

### Steps:

1. **Open Digit.sln in Visual Studio 2022**

2. **Add the project:**
   - Right-click on Solution 'Digit' in Solution Explorer
   - Select **Add** ? **Existing Project...**
   - Navigate to `ApertureCore\ApertureCore.vcxproj`
   - Click **Open**

3. **Configure build order (if needed):**
   - Right-click Solution ? **Project Dependencies**
   - Select projects that depend on ApertureCore
   - Check `ApertureCore` in the dependencies list

4. **Build:**
   - Right-click `ApertureCore` ? **Build**
   - Or build entire solution: **Build** ? **Build Solution** (Ctrl+Shift+B)

### Advantages:
- ? Simple and quick
- ? Visual Studio handles GUID generation
- ? No manual file editing

---

## ? Method 2: PowerShell Script (Automated)

### Prerequisites:
- Native VS project files created (already done!)
  - `ApertureCore/ApertureCore.vcxproj`
  - `ApertureCore/ApertureCore.vcxproj.filters`

### Steps:

1. **Run the script:**
   ```powershell
   cd C:\Users\vovch\source\repos\Vovchek\Digit
   .\ApertureCore\add-to-solution.ps1
   ```

2. **Open Digit.sln in Visual Studio**

3. **Verify:**
   - ApertureCore should appear in Solution Explorer
   - Build the project to confirm

### Advantages:
- ? Automated
- ? Creates backup (Digit.sln.backup)
- ? Reproducible

---

## ? Method 3: Manual Solution File Edit

### Steps:

1. **Close Visual Studio** (if open)

2. **Backup Digit.sln:**
   ```powershell
   Copy-Item Digit.sln Digit.sln.backup
   ```

3. **Open Digit.sln in text editor**

4. **Find the last `Project` section** (should look like this):
   ```
   Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Tests", "Tests\Tests.vcxproj", "{9F4C3E94-ADB9-496B-91C7-21012269E892}"
   EndProject
   ```

5. **Add ApertureCore entry after it:**
   ```
   Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "ApertureCore", "ApertureCore\ApertureCore.vcxproj", "{GENERATE-NEW-GUID}"
   EndProject
   ```

6. **Generate a GUID:**
   ```powershell
   [guid]::NewGuid().ToString().ToUpper()
   ```
   Copy the result and replace `{GENERATE-NEW-GUID}`

7. **Save Digit.sln**

8. **Open in Visual Studio**

### Advantages:
- ? Full control
- ? No scripts needed
- ? Good for understanding solution structure

---

## Project Configuration

### Current Settings (ApertureCore.vcxproj):

- **Platform:** x64
- **Configurations:** Debug, Release
- **Output Type:** Static Library (.lib)
- **C++ Standard:** C++17
- **Warnings:** Level 4, Treat as Errors
- **Include Paths:** `$(ProjectDir)include`

### Output Locations:

```
Debug:   $(SolutionDir)x64\Debug\ApertureCore.lib
Release: $(SolutionDir)x64\Release\ApertureCore.lib
```

This matches the Digit solution's output structure.

---

## Using ApertureCore in Other Projects

Once added to the solution, other projects can use it:

### 1. Add Project Reference:

In Visual Studio:
- Right-click `Digit` (or other project) ? **Add** ? **Reference**
- Check `ApertureCore`
- Click **OK**

### 2. Add Include Directory:

In project properties:
- **C/C++** ? **General** ? **Additional Include Directories**
- Add: `$(SolutionDir)ApertureCore\include`

### 3. Use in Code:

```cpp
#include <aperturecore/geometry/Point.h>
#include <aperturecore/geometry/Bounds.h>
#include <aperturecore/visibility/TypeLimits.h>

using namespace aperture;

// Use Point class
Point p{10.0, 20.0};
double dist = p.magnitude();

// Use TypeLimits
TypeLimits type = TypeLimits::APERTURE;
```

---

## Building

### Build ApertureCore Only:

```
Visual Studio: Right-click ApertureCore ? Build
MSBuild:       msbuild ApertureCore\ApertureCore.vcxproj /p:Configuration=Debug
```

### Build Entire Solution:

```
Visual Studio: Build ? Build Solution (Ctrl+Shift+B)
MSBuild:       msbuild Digit.sln /p:Configuration=Debug
```

---

## Testing

### Tests are in separate CMake project:

The test suite uses Google Test and is built via CMake:

```powershell
cd ApertureCore
.\build.ps1 Debug --test
```

This runs **75 tests** for Point and Bounds classes.

**Note:** Tests are NOT in the VS solution (by design, to keep it clean).

---

## Hybrid Build Approach

You can use BOTH build systems:

### For Development:
- **Visual Studio** - Main Digit solution with ApertureCore
- Quick iteration, debugging, IntelliSense

### For Testing:
- **CMake** - Run comprehensive test suite
- Cross-platform verification

```powershell
# Development in VS
# (work on code, build, debug)

# Testing via CMake
cd ApertureCore
.\build.ps1 Debug --test
```

---

## Troubleshooting

### "Cannot open include file"

**Solution:** Add include directory to project settings:
```
$(SolutionDir)ApertureCore\include
```

### "Unresolved external symbol"

**Solution:** Add project reference or link to .lib:
```
$(SolutionDir)x64\$(Configuration)\ApertureCore.lib
```

### "Project failed to load"

**Solution:** 
1. Check .vcxproj file exists at specified path
2. Verify GUID is unique
3. Check XML is well-formed

### Build order issues

**Solution:** Set up project dependencies:
- Right-click Solution ? **Project Dependencies**
- Configure dependency graph

---

## Migration Path

### Phase 1: Side-by-side (Current)
```
Digit.sln
??? Digit.vcxproj (existing)
??? InterfSolver.vcxproj (existing, MFC-based)
??? ApertureCore.vcxproj (new, modern C++)
??? Tests.vcxproj (existing)
```

### Phase 2: Integration
```
Digit.exe can use BOTH:
- InterfSolver.dll (legacy)
- ApertureCore.lib (new)
```

### Phase 3: Replacement
```
Digit.exe uses only:
- ApertureCore.lib (modern)
InterfSolver.dll deprecated
```

---

## File Organization

```
Digit/
??? Digit.sln                    # Solution file (MODIFIED)
??? Digit.vcxproj               # Main app
??? InterfSolver/
?   ??? InterfSolver.vcxproj    # Legacy DLL
??? ApertureCore/               # NEW!
?   ??? ApertureCore.vcxproj    # Native VS project
?   ??? CMakeLists.txt          # CMake (for testing)
?   ??? include/
?   ?   ??? aperturecore/
?   ?       ??? geometry/
?   ?       ??? visibility/
?   ??? src/
?   ?   ??? geometry/
?   ?   ??? visibility/
?   ??? tests/                  # CMake-based tests
??? Tests/
    ??? Tests.vcxproj           # Existing tests
```

---

## Recommended Approach

**For your situation:**

1. ? **Use Method 1 (VS GUI)** - Quickest and safest
2. ? Keep CMake for testing
3. ? Build both in VS for integration work
4. ? Use CMake `build.ps1 --test` for comprehensive testing

---

## Summary

| Method | Difficulty | Speed | Best For |
|--------|-----------|-------|----------|
| VS GUI | ? Easy | ? Fast | Quick integration |
| PowerShell | ?? Medium | ?? Very Fast | Automation |
| Manual Edit | ??? Hard | ? Slow | Learning/Control |

**Recommendation:** Use **Method 1 (VS GUI)** now, then automate with scripts later if needed.

---

## Next Steps

After adding to solution:

1. ? Build ApertureCore in VS
2. ? Verify output: `x64\Debug\ApertureCore.lib`
3. ? Run CMake tests: `.\build.ps1 Debug --test`
4. ? Add reference from Digit project (when ready to use)
5. ? Start Phase 1 Day 2: Implement Shape hierarchy

---

**Ready to add ApertureCore to Digit.sln!** ??
