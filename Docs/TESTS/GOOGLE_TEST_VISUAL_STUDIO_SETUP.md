# Setting Up Google Test in Visual Studio for Tests Project

## Current Status

Your `Tests` project currently uses an **old Google Test NuGet package** (v1.8.1.7 from 2017) which may not integrate well with modern Visual Studio Test Explorer.

## Recommended: Upgrade to Modern Google Test

### Option 1: Use NuGet Package Manager (Recommended - Easy)

1. **Open Visual Studio**
2. **Right-click on the `Tests` project** in Solution Explorer
3. **Select "Manage NuGet Packages..."**
4. **Go to "Installed" tab**
5. **Uninstall the old package:**
   - Find `Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn`
   - Click "Uninstall"

6. **Go to "Browse" tab**
7. **Search for:** `Microsoft.googletest.v143.static.rt-dyn`
8. **Install** the latest version (should be ~1.14.0)

9. **Also install Test Adapter:**
   - Search for: `GoogleTestAdapter`
   - Install the latest version

10. **Rebuild your Tests project** (Ctrl+Shift+B on Tests project)

### Option 2: Manual Update via packages.config

1. Open `Tests/packages.config`
2. Replace the existing line with:

```xml
<?xml version="1.0" encoding="utf-8"?>
<packages>
  <package id="Microsoft.googletest.v143.static.rt-dyn" version="1.14.0" targetFramework="native" />
</packages>
```

3. **In Visual Studio:**
   - Right-click on `Tests` project
   - Select "Restore NuGet Packages"
   - Rebuild the project

---

## Verify Test Explorer Setup

After updating Google Test:

### 1. Enable Test Explorer

**Menu:** `Test` ? `Test Explorer` (or press `Ctrl+E, T`)

### 2. Build the Tests Project

Press `Ctrl+Shift+B` while `Tests` project is selected

### 3. Check Test Explorer

You should see all your tests listed in categories:
- `InterfSolver.Tools.XYEllipseTest`
- `InterfSolver.Tools.XYRectTest`
- `InterfSolver.Tools.XYPolygonTest`
- etc.

### 4. Run Tests

**Options:**
- Click "Run All" (green play button in Test Explorer)
- Right-click specific test ? "Run"
- Right-click test file in Solution Explorer ? "Run Tests"

---

## Troubleshooting

### Tests Don't Appear in Test Explorer

**Try these in order:**

1. **Clean and Rebuild:**
   ```
   Build ? Clean Solution
   Build ? Rebuild Tests
   ```

2. **Check Test Adapter:**
   - `Tools` ? `Options` ? `Test` ? `Test Adapters`
   - Ensure "GoogleTest Adapter" is enabled

3. **Update Test Adapter:**
   - `Tools` ? `Extensions and Updates`
   - Search for "Test Adapter for Google Test"
   - Update to latest version

4. **Restart Visual Studio** after updating

### Tests Build But Don't Run

1. **Check Platform Target:**
   - Tests project properties ? Configuration ? Platform
   - Should match: `x64` (or `x86` if that's what you use)

2. **Check Configuration:**
   - Active configuration should be `Debug` or `Release`
   - Test Explorer uses active configuration

3. **Verify Google Test Installation:**
   - Solution Explorer ? Tests project ? References
   - Should see `gtest`, `gtest_main` libraries

---

## Running Tests from Command Line (Alternative)

If Test Explorer doesn't work, you can run tests manually:

```powershell
# From Tests project directory
cd Tests
.\x64\Debug\Tests.exe --gtest_output=xml:test_results.xml
```

Or use VSTest:

```powershell
vstest.console.exe Tests\x64\Debug\Tests.exe
```

---

## Recommended Test Explorer Settings

**Tools ? Options ? Test ? General:**

- [x] Automatically run tests after build
- [x] Show additional test context (errors, output)
- [ ] Collapse test groups after run (uncheck for better visibility)

**Tools ? Options ? Test ? Google Test:**

- [x] Run disabled tests: false
- [x] Shuffle tests: false
- [x] Number of test repetitions: 1

---

## Quick Reference: Test Explorer Shortcuts

| Action | Shortcut |
|--------|----------|
| Open Test Explorer | `Ctrl+E, T` |
| Run All Tests | `Ctrl+R, A` |
| Run Last Test Run | `Ctrl+R, L` |
| Debug All Tests | `Ctrl+R, Ctrl+A` |
| Debug Selected Tests | `Ctrl+R, Ctrl+T` |

---

## Expected Result After Setup

Once configured correctly, you should be able to:

? See all 75+ passing tests in Test Explorer  
? Run individual tests by clicking them  
? Run test categories (e.g., all XYEllipseTest tests)  
? Debug tests with breakpoints  
? See test output and failure messages  
? Filter tests by name, outcome, or trait  

---

## Current Test Count

Based on your codebase:
- **XYEllipseTest:** ~40 tests
- **XYRectTest:** ~30 tests  
- **XYPolygonTest:** ~35 tests
- **XYShapeTest:** ~10 tests
- **CalcContourTest:** ~15 tests
- **isPupilTest:** ~10 tests

**Total:** ~140 tests

All should appear in Test Explorer after proper setup!

---

## Need Help?

If you encounter issues:

1. Check Visual Studio version (2019/2022 recommended)
2. Verify NuGet package restore is enabled
3. Try the "Clean Solution" approach
4. Check Output window for build errors

Let me know what you see in Test Explorer after updating!
