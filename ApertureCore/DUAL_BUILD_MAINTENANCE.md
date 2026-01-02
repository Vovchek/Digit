# Dual Build System Maintenance Guide

## Overview

ApertureCore uses **two build systems** that share the same source files:

| Build System | Purpose | Maintained By |
|--------------|---------|---------------|
| **CMake** | Testing, cross-platform | Primary (source of truth) |
| **Visual Studio** | Integration with Digit | Synced from CMake |

---

## ?? **Recommended Workflow**

### Principle: **CMake is Source of Truth**

1. ? **Add files** to CMakeLists.txt first
2. ? **Run sync script** to update .vcxproj
3. ? **Commit both** to git

This ensures:
- CMake builds work (tests pass)
- VS builds work (Digit integration)
- Both stay synchronized

---

## ?? **Workflow for Adding New Files**

### Step 1: Create the Files

```powershell
# Create new source and header
New-Item -Path "include\aperturecore\geometry\Ellipse.h"
New-Item -Path "src\geometry\Ellipse.cpp"
```

### Step 2: Update CMakeLists.txt

Edit `CMakeLists.txt`:

```cmake
set(APERTURE_SOURCES
    # Existing files...
    src/geometry/Point.cpp
    src/geometry/Bounds.cpp
    
    # NEW FILES
    src/geometry/Ellipse.cpp
    
    # ...
)
```

Headers are **auto-discovered** by CMake (in `include/` directory), so you don't need to list them.

### Step 3: Sync to Visual Studio

```powershell
cd ApertureCore
.\sync-builds.ps1
```

**Output:**
```
ApertureCore Build System Sync
===============================

Found in CMakeLists.txt:
  Source files: 4
  Header files: 0

Auto-discovered headers: 5

Synchronization Analysis:

  Missing in .vcxproj (source files):
    + src\geometry\Ellipse.cpp
    
  Missing in .vcxproj (headers):
    + include\aperturecore\geometry\Ellipse.h

  Added: src\geometry\Ellipse.cpp
  Added: include\aperturecore\geometry\Ellipse.h
  
  Backup created: ApertureCore.vcxproj.backup
  ? Saved ApertureCore.vcxproj

Done!
```

### Step 4: Update Filters (Optional)

For better organization in Visual Studio, manually update `ApertureCore.vcxproj.filters`:

```xml
<ClCompile Include="src\geometry\Ellipse.cpp">
  <Filter>Source Files\geometry</Filter>
</ClCompile>

<ClInclude Include="include\aperturecore\geometry\Ellipse.h">
  <Filter>Header Files\geometry</Filter>
</ClInclude>
```

### Step 5: Verify Both Builds

**CMake:**
```powershell
.\build.ps1 Debug --test
```

**Visual Studio:**
```
Open Digit.sln
Build ? Build ApertureCore
```

### Step 6: Commit

```bash
git add CMakeLists.txt
git add ApertureCore.vcxproj
git add ApertureCore.vcxproj.filters
git add include/aperturecore/geometry/Ellipse.h
git add src/geometry/Ellipse.cpp
git commit -m "feat(geometry): add Ellipse class"
```

---

## ?? **Sync Script Usage**

### Dry Run (Check Only)

```powershell
.\sync-builds.ps1 -DryRun
```

Shows what would change without modifying files.

### Apply Changes

```powershell
.\sync-builds.ps1
```

Automatically:
- Adds missing files to .vcxproj
- Removes extra files from .vcxproj
- Creates backup (.vcxproj.backup)

---

## ?? **What's NOT Synchronized**

### Tests (Intentional)

Tests are **only in CMake**, not in Visual Studio project:

```
tests/
??? geometry/
?   ??? PointTest.cpp      ? CMake only
?   ??? BoundsTest.cpp     ? CMake only
??? CMakeLists.txt
```

**Why?** 
- Tests use Google Test (CMake-based)
- Keeps VS project clean
- Run tests via `build.ps1 --test`

### Build Configuration

Compiler flags, warnings, etc. must be **manually synchronized**:

| Setting | CMake | Visual Studio |
|---------|-------|---------------|
| C++ Standard | `set(CMAKE_CXX_STANDARD 17)` | `<LanguageStandard>stdcpp17</LanguageStandard>` |
| Warnings | `add_compile_options(/W4 /WX)` | `<WarningLevel>Level4</WarningLevel>` |
| Include Dirs | `target_include_directories()` | `<AdditionalIncludeDirectories>` |

**Manual sync required** when changing these!

---

## ?? **Common Pitfalls**

### Pitfall 1: Forgetting to Sync

**Problem:**
```
CMakeLists.txt: Added Shape.cpp
.vcxproj:        Still missing Shape.cpp
Result:          CMake builds, VS doesn't!
```

**Solution:**
```powershell
.\sync-builds.ps1
```

### Pitfall 2: Editing .vcxproj Directly

**Problem:**
```
User adds file to .vcxproj manually
CMakeLists.txt still doesn't have it
Result:          VS builds, CMake doesn't!
```

**Solution:**
- Always edit CMakeLists.txt first
- Then run sync script

### Pitfall 3: Merge Conflicts

**Problem:**
```
Both CMakeLists.txt and .vcxproj modified in different branches
Merge conflict!
```

**Solution:**
```bash
# Resolve CMakeLists.txt first
git checkout --theirs CMakeLists.txt
# Or manually merge

# Then regenerate .vcxproj
.\sync-builds.ps1

git add CMakeLists.txt ApertureCore.vcxproj
```

---

## ?? **File Organization**

### Naming Convention

| Type | Location | Example |
|------|----------|---------|
| Public Headers | `include/aperturecore/` | `Point.h`, `Shape.h` |
| Source Files | `src/` | `Point.cpp`, `Shape.cpp` |
| Tests | `tests/` | `PointTest.cpp` (CMake only) |
| Docs | Root | `README.md`, `DESIGN.md` |

### Filters in Visual Studio

Organize by category, not directory:

```
Source Files
??? geometry
?   ??? Point.cpp
?   ??? Bounds.cpp
?   ??? Ellipse.cpp
??? visibility
    ??? TypeLimits.cpp

Header Files
??? geometry
?   ??? Point.h
?   ??? Bounds.h
?   ??? Ellipse.h
??? visibility
    ??? TypeLimits.h
```

---

## ?? **Automated Workflow (Git Hook)**

### Pre-Commit Hook

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash
# Auto-sync before commit

if git diff --cached --name-only | grep -q "CMakeLists.txt"; then
    echo "CMakeLists.txt changed, syncing to .vcxproj..."
    cd ApertureCore
    powershell -ExecutionPolicy Bypass -File sync-builds.ps1
    
    # Stage the updated .vcxproj
    git add ApertureCore.vcxproj
    
    echo "? Build systems synchronized"
fi
```

This **automatically syncs** when you commit changes to CMakeLists.txt!

---

## ?? **Maintenance Checklist**

### Daily Development
- [ ] Edit source files (both builds see changes automatically)
- [ ] Build via CMake **or** VS (whichever is convenient)

### When Adding Files
- [ ] Add to CMakeLists.txt
- [ ] Run `sync-builds.ps1`
- [ ] Update .vcxproj.filters (optional, for organization)
- [ ] Verify both builds work
- [ ] Commit all changes

### When Changing Build Config
- [ ] Update CMakeLists.txt
- [ ] **Manually** update .vcxproj settings
- [ ] Document in commit message

### Before Pull Request
- [ ] Run sync script
- [ ] CMake build + tests pass
- [ ] VS build succeeds
- [ ] Commit .vcxproj changes

---

## ?? **Verification Commands**

### Check Sync Status

```powershell
.\sync-builds.ps1 -DryRun
```

Should output: "? All files in sync"

### Build Both Systems

```powershell
# CMake
.\build.ps1 Debug --test

# Visual Studio (from solution directory)
msbuild Digit.sln /t:ApertureCore /p:Configuration=Debug
```

Both should succeed!

---

## ?? **Best Practices**

### DO ?

1. **Edit CMakeLists.txt first** - It's the source of truth
2. **Run sync script** after adding/removing files
3. **Commit both** CMakeLists.txt and .vcxproj together
4. **Test both builds** before pushing

### DON'T ?

1. **Don't edit .vcxproj directly** - Use sync script
2. **Don't forget .vcxproj.filters** - Keep VS organized
3. **Don't commit without syncing** - Breaks other developers' builds
4. **Don't modify build configs independently** - Keep them aligned

---

## ?? **Summary**

| Aspect | Strategy |
|--------|----------|
| **Primary** | CMake (tests, cross-platform) |
| **Secondary** | Visual Studio (Digit integration) |
| **Sync Method** | PowerShell script (`sync-builds.ps1`) |
| **Workflow** | Edit CMake ? Sync ? Commit both |
| **Tests** | CMake only (intentional) |
| **Build Config** | Manual sync required |

---

## ?? **Quick Reference**

```powershell
# Add new file
1. Create file (Shape.cpp, Shape.h)
2. Edit CMakeLists.txt (add to APERTURE_SOURCES)
3. .\sync-builds.ps1
4. Verify builds
5. Commit

# Check sync status
.\sync-builds.ps1 -DryRun

# Build both systems
.\build.ps1 Debug --test                    # CMake
msbuild ..\Digit.sln /t:ApertureCore        # VS
```

---

**Maintaining dual build systems is manageable with discipline and automation!** 

The sync script handles 90% of the work. Just remember: **CMake first, then sync!** ??
