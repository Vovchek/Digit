# Adding ApertureCore to Digit Solution - Quick Start

## Yes, You Can! ?

ApertureCore can be added to the Digit Visual Studio solution in **under 2 minutes**.

---

## Fastest Method (30 seconds)

### Option A: Visual Studio GUI

1. Open `Digit.sln` in Visual Studio
2. Right-click Solution ? **Add** ? **Existing Project**
3. Select `ApertureCore\ApertureCore.vcxproj`
4. Done! ?

---

## What You Get

### In Solution Explorer:
```
Solution 'Digit'
??? Digit
??? EngLangD
??? RusLangD
??? MGTools
??? InterfSolver
??? Tests
??? ApertureCore        ? NEW!
    ??? Header Files
    ?   ??? geometry
    ?   ?   ??? Point.h
    ?   ?   ??? Bounds.h
    ?   ??? visibility
    ?       ??? TypeLimits.h
    ??? Source Files
    ?   ??? geometry
    ?   ?   ??? Point.cpp
    ?   ?   ??? Bounds.cpp
    ?   ??? visibility
    ?       ??? TypeLimits.cpp
    ??? Documentation
        ??? README.md
        ??? DESIGN.md
        ??? ...
```

### Build Output:
```
x64\Debug\ApertureCore.lib      (Debug build)
x64\Release\ApertureCore.lib    (Release build)
```

---

## Files Created

Ready-to-use Visual Studio project files:

1. ? `ApertureCore/ApertureCore.vcxproj` - Native VS project
2. ? `ApertureCore/ApertureCore.vcxproj.filters` - Folder structure
3. ? `ApertureCore/add-to-solution.ps1` - Automation script
4. ? `ApertureCore/INTEGRATION_GUIDE.md` - Complete guide

---

## Project Settings

- **Platform:** x64 (matches Digit)
- **C++ Standard:** C++17
- **Output Type:** Static Library (.lib)
- **Warnings:** Level 4, Treat as Error
- **No MFC:** Pure STL

---

## Using in Other Projects

After adding to solution:

### Add Reference:
1. Right-click `Digit` project ? **Add** ? **Reference**
2. Check `ApertureCore` ? **OK**

### Add Include Path:
**Project Properties** ? **C/C++** ? **General** ? **Additional Include Directories**:
```
$(SolutionDir)ApertureCore\include
```

### Use in Code:
```cpp
#include <aperturecore/geometry/Point.h>

using namespace aperture;
Point p{10.0, 20.0};
```

---

## Testing

Tests use CMake (separate from VS solution):

```powershell
cd ApertureCore
.\build.ps1 Debug --test

# Output: 75/75 tests passed ?
```

---

## Build Strategies

### Strategy 1: Development in VS
- Add to solution ?
- Edit code in VS
- Build with Ctrl+Shift+B
- Debug with F5

### Strategy 2: Testing via CMake
- Keep tests in CMake
- Run `build.ps1 --test`
- Full test coverage

### Strategy 3: Hybrid (Recommended!)
- **Development:** Visual Studio (fast, IntelliSense)
- **Testing:** CMake (comprehensive test suite)

---

## Next Steps

1. **Add to solution** (30 seconds)
2. **Build ApertureCore** (Ctrl+Shift+B)
3. **Verify output:** Check `x64\Debug\ApertureCore.lib` exists
4. **Continue Phase 1:** Implement Shape hierarchy

---

## Troubleshooting

**"Project failed to load"**
- Verify paths in .vcxproj are relative to Digit directory
- Check .vcxproj file is well-formed XML

**"Cannot find include files"**
- Add `$(SolutionDir)ApertureCore\include` to include directories

**Still have CMake?**
- Yes! CMake is for testing (separate from VS build)
- Use whichever is convenient

---

## Why This Works

**ApertureCore is dual-build:**
- **CMake:** For testing, cross-platform, modern workflow
- **Native VS:** For integration with existing Digit solution

Both build systems work with the same source files!

---

## File Structure

```
ApertureCore/
??? ApertureCore.vcxproj         ? Visual Studio project
??? ApertureCore.vcxproj.filters ? VS folder structure
??? CMakeLists.txt               ? CMake build (for tests)
??? build.ps1                    ? CMake build script
??? include/                     ? Headers (shared by both)
??? src/                         ? Source (shared by both)
??? tests/                       ? CMake-based tests
```

**Both build systems share the same code!** ??

---

## Summary

| Question | Answer |
|----------|--------|
| Can I add to Digit.sln? | ? **Yes!** |
| How long? | ? **30 seconds** |
| Will it break CMake? | ? **No!** Both work |
| Ready to use? | ? **Yes!** Files already created |

---

**Go ahead and add it!** The files are ready. ??

**Recommended action:**
```
1. Open Digit.sln
2. Add ? Existing Project ? ApertureCore.vcxproj
3. Build!
```
