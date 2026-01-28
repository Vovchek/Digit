# Quick Start: Running Tests in Visual Studio

## FASTEST METHOD (3 Steps)

### Step 1: Update Google Test Package

**In Visual Studio:**
1. Right-click `Tests` project ? **Manage NuGet Packages**
2. **Installed** tab ? Find `Microsoft.googletest.v140...` ? **Uninstall**
3. **Browse** tab ? Search `Microsoft.googletest.v143.static.rt-dyn` ? **Install**
4. Click **OK** on any prompts

### Step 2: Open Test Explorer

**Press:** `Ctrl+E, T`

Or: **Menu** ? `Test` ? `Test Explorer`

### Step 3: Build and Run

1. **Build Tests:** Right-click `Tests` project ? **Build** (or `Ctrl+Shift+B`)
2. **In Test Explorer:** Click **"Run All"** (green play button)

? **Done!** You should see ~140 tests running!

---

## What You'll See

**Test Explorer will show:**

```
? InterfSolver
   ?? Tools
   ?  ?? XYEllipseTest (40 tests)
   ?  ?  ?? ? DefaultConstructor
   ?  ?  ?? ? ParameterizedConstructor
   ?  ?  ?? ? isInside_Center
   ?  ?  ?? ? Perimeter_Circle
   ?  ?  ?? ...
   ?  ?? XYRectTest (30 tests)
   ?  ?? XYPolygonTest (35 tests)
   ?  ?? CalcContourTest (15 tests)
   ?  ?? isPupilTest (10 tests)
```

---

## Running Specific Tests

### Run One Test
1. Find test in Test Explorer
2. Right-click ? **Run**

### Run Test Suite
1. Right-click `XYEllipseTest` ? **Run**

### Debug a Test
1. Right-click test ? **Debug**
2. Set breakpoints in test code
3. Debugger stops at breakpoints

---

## Common Issues & Fixes

### Issue: "No tests found"

**Fix:**
```
1. Build ? Clean Solution
2. Build ? Rebuild Tests
3. Test Explorer ? Refresh (circular arrow icon)
```

### Issue: "Tests are grayed out"

**Fix:**
- Check active configuration (Debug/Release)
- Match platform (x64 vs x86)

### Issue: "gtest.h not found" error

**Fix:**
- Reinstall Google Test NuGet package
- Check project includes gtest/gtest_main libraries

---

## Alternative: Command Line

If Test Explorer doesn't work:

```powershell
# Run tests directly
cd Tests\x64\Debug
.\Tests.exe

# Run with output
.\Tests.exe --gtest_output=xml:results.xml

# Run specific test
.\Tests.exe --gtest_filter=XYEllipseTest.isInside_Center
```

---

## Expected Output

**Console (if running from command line):**
```
[==========] Running 140 tests from 6 test suites.
[----------] Global test environment set-up.
[----------] 40 tests from XYEllipseTest
[ RUN      ] XYEllipseTest.DefaultConstructor
[       OK ] XYEllipseTest.DefaultConstructor (0 ms)
[ RUN      ] XYEllipseTest.ParameterizedConstructor
[       OK ] XYEllipseTest.ParameterizedConstructor (0 ms)
...
[==========] 140 tests from 6 test suites ran. (243 ms total)
[  PASSED  ] 140 tests.
```

**Test Explorer:**
- Green checkmarks for passing tests
- Red X for failing tests
- Test execution time
- Output/error messages for failures

---

## That's It!

You're now running Google Test in Visual Studio! ??

**Next:** Run your tests to verify all code is working correctly.
