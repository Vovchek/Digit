# Visual Studio Test Integration Guide

**Last Updated:** 2024  
**Status:** Comprehensive troubleshooting and setup guide

---

## Overview

This guide addresses issues with running and debugging ApertureCore tests from within Visual Studio. While command-line testing works perfectly, VS integration requires additional setup.

---

## Current Status

### ✅ What Works

1. **Command-Line Testing** (Fully Working)
   ```powershell
   # Quick test run
   cd ApertureCore
   .\run-tests.ps1
   
   # Single suite
   .\run-single-test.ps1 -Suite geometry
   
   # Specific test
   .\run-single-test.ps1 -Suite geometry -Filter "PolygonTest.Area"
   
   # Watch mode
   .\watch-tests.ps1
   ```

2. **CMake Build** (Fully Working)
   ```powershell
   cmake -B build -S . -DAPERTURE_BUILD_TESTS=ON
   cmake --build build --config Debug
   cd build
   ctest --verbose
   ```

3. **Direct Execution** (Fully Working)
   ```powershell
   .\build\Debug\geometry_tests.exe
   .\build\Debug\visibility_tests.exe
   ```

### ❌ What Doesn't Work Yet

1. ❌ Test Explorer discovery in Visual Studio
2. ❌ "Run All Tests" button in Test Explorer
3. ❌ Individual test execution from Test Explorer
4. ❌ Debugging tests from Test Explorer UI
5. ❌ Test targets visible in Solution Explorer

---

## Solutions & Workarounds

### Solution 1: Use Improved sync-builds.ps1 ✅

**Status:** IMPLEMENTED (see Issue #3 fix)

The enhanced `sync-builds.ps1` now:
- Parses `tests/CMakeLists.txt` subdirectory
- Extracts test executables (geometry_tests, visibility_tests)
- Adds test source files to `.vcxproj`
- Organizes them in "Tests" filter folders

**Run this after any CMakeLists.txt changes:**
```powershell
cd ApertureCore
.\sync-builds.ps1
```

**Expected output:**
```
Found test: geometry_tests (5 files)
Found test: visibility_tests (3 files)
  Added: tests\geometry\PointTest.cpp
  Added: tests\geometry\BoundsTest.cpp
  ...
```

---

### Solution 2: Install Google Test Adapter

**For Test Explorer to work, you need the Google Test Adapter extension.**

#### Step 1: Check if Installed

1. Open Visual Studio
2. **Extensions** → **Manage Extensions**
3. Search for "Test Adapter for Google Test"
4. Check if installed

#### Step 2: Install if Missing

1. In Extensions window, click "Online"
2. Search "Google Test Adapter"
3. Find "Test Adapter for Google Test" by Christian Soltenborn
4. Click "Download"
5. Close Visual Studio to install
6. Reopen VS

#### Step 3: Configure Adapter

1. **Tools** → **Options**
2. Navigate to **Test Adapter for Google Test**
3. Configure:
   - **Test discovery regex:** `.*tests?\.exe$`
   - **Test execution directory:** `$(ExecutableDir)`
   - **Working directory:** `$(SolutionDir)`
   - **Batch for test execution:** `Enabled`

---

### Solution 3: Add Test Executables to Solution

Even if Test Explorer doesn't work, you can add test executables as projects.

#### Option A: Add as External Tools

1. **Tools** → **External Tools...**
2. Click "Add"
3. Configure:
   - **Title:** Run Geometry Tests
   - **Command:** `$(SolutionDir)ApertureCore\build\Debug\geometry_tests.exe`
   - **Initial directory:** `$(SolutionDir)ApertureCore\build\Debug`
   - **Use Output window:** Checked
4. Repeat for visibility_tests

**Usage:** Tools → Run Geometry Tests

#### Option B: Create Custom Build Targets

Add to `ApertureCore.vcxproj` (already done by sync-builds.ps1):

```xml
<Target Name="RunGeometryTests" AfterTargets="Build">
  <Exec Command="&quot;$(SolutionDir)ApertureCore\build\Debug\geometry_tests.exe&quot;" 
        IgnoreExitCode="true" />
</Target>
```

---

### Solution 4: Use CMake Integration Mode

Visual Studio has better test discovery when using CMake mode.

#### Step 1: Open as CMake Project

1. Close current solution
2. **File** → **Open** → **CMake...**
3. Select `ApertureCore/CMakeLists.txt`
4. Wait for CMake configuration

#### Step 2: Select Test Target

1. In toolbar, find "Select Startup Item" dropdown
2. You should see:
   - `geometry_tests.exe`
   - `visibility_tests.exe`
3. Select one

#### Step 3: Build and Run

1. **Build** → **Build All**
2. **Debug** → **Start Without Debugging** (Ctrl+F5)

**Pros:**
- Better CMake integration
- Test targets automatically discovered
- Can set breakpoints and debug

**Cons:**
- Separate from main Digit.sln
- Need to switch between solutions

---

### Solution 5: Debugging Individual Tests

Even without Test Explorer, you can debug specific tests.

#### Step 1: Build Test Executable

```powershell
cd ApertureCore
cmake --build build --config Debug
```

#### Step 2: Attach Debugger

1. In Visual Studio, open source file (e.g., `PolygonTest.cpp`)
2. Set breakpoint in test case
3. **Debug** → **Attach to Process...**
4. Run test from command line:
   ```powershell
   .\build\Debug\geometry_tests.exe --gtest_filter="PolygonTest.Area"
   ```
5. Before it finishes, attach to `geometry_tests.exe` process

**Better way: Use debugger launch**

#### Step 3: Configure Debugger in VS

1. Right-click `ApertureCore` project
2. **Properties** → **Debugging**
3. Set:
   - **Command:** `$(SolutionDir)ApertureCore\build\Debug\geometry_tests.exe`
   - **Command Arguments:** `--gtest_filter=PolygonTest.Area`
   - **Working Directory:** `$(SolutionDir)ApertureCore\build\Debug`
4. Apply

Now you can:
- Set breakpoints in test code
- Press **F5** to debug
- Test runs with debugger attached

---

### Solution 6: Use launch.vs.json (Recommended)

Create `.vs/launch.vs.json` in ApertureCore directory:

```json
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "ApertureCore.vcxproj",
      "name": "Geometry Tests",
      "args": [ "--gtest_filter=*" ],
      "currentDir": "${workspaceRoot}\\build\\Debug",
      "program": "${workspaceRoot}\\build\\Debug\\geometry_tests.exe"
    },
    {
      "type": "default",
      "project": "ApertureCore.vcxproj",
      "name": "Visibility Tests",
      "args": [ "--gtest_filter=*" ],
      "currentDir": "${workspaceRoot}\\build\\Debug",
      "program": "${workspaceRoot}\\build\\Debug\\visibility_tests.exe"
    },
    {
      "type": "default",
      "project": "ApertureCore.vcxproj",
      "name": "Polygon Tests Only",
      "args": [ "--gtest_filter=PolygonTest.*" ],
      "currentDir": "${workspaceRoot}\\build\\Debug",
      "program": "${workspaceRoot}\\build\\Debug\\geometry_tests.exe"
    }
  ]
}
```

**Usage:**
1. Click dropdown next to "Start" button (green arrow)
2. Select "Geometry Tests" or other configuration
3. Press F5 to debug

---

## Recommended Workflow

### During Development (Best Experience)

**Option 1: PowerShell Scripts (Fastest)**
```powershell
# Terminal 1: Watch mode
cd ApertureCore
.\watch-tests.ps1

# Terminal 2: Manual runs when needed
.\run-single-test.ps1 -Suite geometry -Filter "PolygonTest.*"
```

**Option 2: VS Debugger for Specific Tests**
1. Set breakpoints in test file
2. Use `launch.vs.json` configuration (see Solution 6)
3. Select test configuration from dropdown
4. Press F5

**Option 3: Hybrid**
- Use PowerShell for quick test runs
- Switch to VS debugger when test fails
- Set breakpoint, run specific test with F5

### For Full Test Suite

```powershell
# Run all tests
.\run-tests.ps1

# Run with verbose output
.\run-tests.ps1 -Verbose

# Run specific suite
.\run-tests.ps1 -Filter geometry_tests
```

---

## Troubleshooting

### Problem: Test Explorer Shows No Tests

**Diagnosis:**
```powershell
# Check if test executables exist
dir build\Debug\*tests.exe

# Try running directly
.\build\Debug\geometry_tests.exe --gtest_list_tests
```

**Solutions:**
1. Install Google Test Adapter extension
2. Rebuild solution
3. **Test** → **Test Explorer** → Refresh
4. Check **Tools** → **Options** → **Test Adapter for Google Test** settings

### Problem: Tests Don't Build

**Diagnosis:**
```powershell
# Check CMake configuration
cd ApertureCore
cmake -B build -S . -DAPERTURE_BUILD_TESTS=ON

# Check if tests are in CMakeLists
Get-Content tests\CMakeLists.txt
```

**Solutions:**
1. Run `sync-builds.ps1` to add tests to .vcxproj
2. Rebuild solution
3. Check build output for errors

### Problem: Can't Debug Tests

**Solution:** Use `launch.vs.json` (see Solution 6)

1. Create `.vs/launch.vs.json`
2. Add test configurations
3. Select from dropdown
4. Press F5

### Problem: Tests Pass in Command Line, Fail in VS

**Likely cause:** Working directory mismatch

**Solution:** Set working directory in launch configuration:
```json
"currentDir": "${workspaceRoot}\\build\\Debug"
```

---

## Quick Reference

### Build Tests
```powershell
cd ApertureCore
cmake -B build -S . -DAPERTURE_BUILD_TESTS=ON
cmake --build build --config Debug
```

### Run All Tests
```powershell
.\run-tests.ps1
```

### Run Single Suite
```powershell
.\run-single-test.ps1 -Suite geometry
```

### Run Specific Test
```powershell
.\run-single-test.ps1 -Suite geometry -Filter "PolygonTest.Area"
```

### Debug Test in VS
1. Create `launch.vs.json` (see Solution 6)
2. Set breakpoints
3. Select configuration from dropdown
4. Press F5

### Sync VS Project
```powershell
.\sync-builds.ps1
```

---

## Future Improvements

### Planned Enhancements

1. **CMakePresets.json** - Modern CMake configuration
2. **GitHub Actions** - Automated CI testing
3. **Test Coverage** - Generate coverage reports
4. **Better VS Integration** - Investigate custom test adapter

### Pending Investigation

- Why Test Explorer doesn't auto-discover tests
- Better CMake/VS integration for hybrid solution
- Custom MSBuild targets for test execution

---

## Summary

**Current Best Practice:**

1. ✅ Use `sync-builds.ps1` to keep .vcxproj in sync
2. ✅ Use PowerShell scripts for quick test runs
3. ✅ Use `launch.vs.json` for debugging specific tests
4. ✅ Use watch mode during active development
5. ⚠️ Test Explorer - nice to have, but not critical

**The command-line workflow is actually very efficient!** Don't feel obligated to use Test Explorer if PowerShell scripts work well for you.

---

**Created:** 2024  
**Last Updated:** After implementing Issue #3-5 fixes  
**Status:** Complete troubleshooting guide
