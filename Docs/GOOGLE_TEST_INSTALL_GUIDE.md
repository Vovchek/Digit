# Google Test Setup - Step by Step with Screenshots Guide

## What You Need

You're currently using an **OLD** Google Test version (1.8.1 from 2017).  
Let's upgrade to the **MODERN** version for better Visual Studio integration.

---

## Method 1: Visual Studio GUI (Easiest) ?

### Step 1: Open NuGet Package Manager

1. **Open Visual Studio**
2. In **Solution Explorer**, find the `Tests` project
3. **Right-click** on `Tests` project
4. Select **"Manage NuGet Packages..."**

   ![NuGet Menu](https://docs.microsoft.com/en-us/nuget/quickstart/media/vs-2019-nuget-package-manager.png)

### Step 2: Uninstall Old Package

1. Click the **"Installed"** tab
2. Find: `Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn`
3. Click on it to select
4. Click **"Uninstall"** button on the right
5. Click **"OK"** on confirmation dialog

### Step 3: Install New Package

1. Click the **"Browse"** tab
2. In search box, type: `googletest`
3. Look for: **`Microsoft.googletest.v143.static.rt-dyn`**
4. Select it from the list
5. On the right panel, click **"Install"**
6. Select latest version (e.g., 1.14.0)
7. Click **"OK"** on preview dialog

### Step 4: Close and Build

1. Close NuGet Package Manager window
2. **Build** the Tests project: Right-click `Tests` ? **Build**
3. Wait for build to complete (should succeed)

? **Done!** Google Test is now updated.

---

## Method 2: Package Manager Console (Advanced)

### Step 1: Open Console

**Menu:** `Tools` ? `NuGet Package Manager` ? `Package Manager Console`

### Step 2: Run Commands

```powershell
# Uninstall old version
Uninstall-Package Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn -Project Tests

# Install new version
Install-Package Microsoft.googletest.v143.static.rt-dyn -Project Tests -Version 1.14.0
```

### Step 3: Build

```
Build ? Rebuild Tests
```

---

## Method 3: Edit packages.config Manually

### Step 1: Edit File

Open `Tests/packages.config` and change:

**FROM:**
```xml
<package id="Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn" version="1.8.1.7" targetFramework="native" />
```

**TO:**
```xml
<package id="Microsoft.googletest.v143.static.rt-dyn" version="1.14.0" targetFramework="native" />
```

### Step 2: Restore Packages

1. Right-click on **Solution** (top level) in Solution Explorer
2. Select **"Restore NuGet Packages"**
3. Wait for restore to complete

### Step 3: Rebuild

```
Build ? Clean Solution
Build ? Rebuild Tests
```

---

## Verify Installation

### Check 1: References

In Solution Explorer:
```
Tests
?? References
?  ?? gtest               ? Should be present
?  ?? gtest_main          ? Should be present
?  ?? Microsoft.googletest...
```

### Check 2: Include Paths

Right-click `Tests` project ? **Properties**:
- **C/C++** ? **General** ? **Additional Include Directories**
- Should contain path to Google Test headers

### Check 3: Build Output

Build the project, check Output window:
```
1>------ Build started: Project: Tests, Configuration: Debug x64 ------
1>  XYEllipseTest.cpp
1>  XYRectTest.cpp
1>  Tests.vcxproj -> C:\...\Debug\Tests.exe
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

? No errors? You're good!

---

## Open Test Explorer

### Quick Way
Press: **`Ctrl+E, T`**

### Menu Way
**Test** ? **Test Explorer**

### What You Should See

After building, Test Explorer shows:
```
Tests (140)
?? Not Run (140)
?  ?? InterfSolver.Tools.XYEllipseTest (40)
?  ?? InterfSolver.Tools.XYRectTest (30)
?  ?? InterfSolver.Tools.XYPolygonTest (35)
?  ?? InterfSolver.Tools.XYShapeTest (10)
?  ?? InterfSolver.Tools.CalcContourTest (15)
?  ?? InterfSolver.Tools.isPupilTest (10)
```

---

## Run Tests

### Run All Tests

In Test Explorer:
1. Click the **"Run All"** button (green play icon with two triangles)
2. Watch tests execute
3. See results in real-time

### Run One Test

1. Expand test hierarchy
2. Find specific test (e.g., `XYEllipseTest.DefaultConstructor`)
3. Right-click ? **"Run"**

### Debug One Test

1. Set breakpoint in test code
2. Right-click test ? **"Debug"**
3. Debugger starts and stops at breakpoint

---

## Troubleshooting

### Problem: "Test Explorer is empty"

**Solutions:**
1. Build the Tests project first
2. Click Refresh button (circular arrows) in Test Explorer
3. Clean Solution, then Rebuild Tests
4. Restart Visual Studio

### Problem: "Cannot find gtest/gtest.h"

**Solutions:**
1. Right-click Tests project ? **Manage NuGet Packages**
2. Verify `Microsoft.googletest.v143.static.rt-dyn` is installed
3. Check version is 1.14.0 or newer
4. Rebuild project

### Problem: "Tests build but won't run"

**Solutions:**
1. Check active configuration (Debug vs Release)
2. Check active platform (x64 vs x86)
3. In Test Explorer, check if tests are grayed out
4. Try running from command line:
   ```
   cd Tests\x64\Debug
   Tests.exe
   ```

### Problem: "Some tests fail"

**This is expected!** Some tests are designed to catch bugs.
- Review test output in Test Explorer
- Click failed test to see error message
- Fix code or update test as needed

---

## Best Practices

### During Development

1. **Keep Test Explorer open** in a side panel
2. **Enable auto-run:** 
   - `Tools` ? `Options` ? `Test`
   - Check "Automatically run tests after build"
3. **Use test filters** to run relevant tests only

### Before Committing

1. **Run All Tests** to ensure nothing broke
2. **Fix any failing tests**
3. **Check test coverage** of new code

### When Debugging

1. **Run single test** instead of all tests
2. **Use Debug mode** to step through code
3. **Check test output** for detailed error messages

---

## Summary

**What You Did:**
? Upgraded Google Test from 1.8.1 ? 1.14.0  
? Configured Test Explorer  
? Can now run tests from Visual Studio GUI  

**What You Can Do Now:**
- Run all tests with one click
- Run individual tests
- Debug tests with breakpoints
- See test results in real-time
- Filter tests by name/status

**Next Steps:**
- Run all tests to verify codebase
- Fix any failing tests
- Add new tests for new features

---

## Quick Reference Card

| Task | Action |
|------|--------|
| Open Test Explorer | `Ctrl+E, T` |
| Run All Tests | Click "Run All" or `Ctrl+R, A` |
| Run Selected | Right-click ? Run |
| Debug Test | Right-click ? Debug or `Ctrl+R, Ctrl+T` |
| Refresh Tests | Click refresh button |
| Filter Tests | Type in search box at top |
| Group Tests | Click "Group By" dropdown |

---

**You're all set! Happy testing! ??**
