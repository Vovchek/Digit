# Visual Studio Setup for Digit Project

## Required Visual Studio Components

### Workloads
- **Desktop development with C++** (`Microsoft.VisualStudio.Workload.NativeDesktop`)

### Individual Components
- **MSVC v142/v143 - VS 2019/2022 C++ x64/x86 build tools**
- **C++ CMake tools for Windows**
- **C++ ATL for latest build tools**
- **C++ MFC for latest build tools**
- **Test Adapter for Google Test**
- **Windows 10 SDK (10.0.19041.0 or later)**
- **Git for Windows**

---

## Installation Options

### Option 1: Using .vsconfig file (Recommended)

1. **Automatic installation:**
   ```bash
   # Copy .vsconfig to repository root
   # Visual Studio will detect it automatically when opening the solution
   ```

2. **Manual import:**
   ```powershell
   # Open Visual Studio Installer
   # Click "More" > "Import configuration"
   # Select the .vsconfig file
   ```

3. **Command line:**
   ```powershell
   vs_installer.exe --config .vsconfig
   ```

### Option 2: Manual Installation

1. Open **Visual Studio Installer**
2. Click **Modify** on your Visual Studio installation
3. Select the following:

   **Workloads tab:**
   - ☑ Desktop development with C++

   **Individual components tab:**
   - ☑ MSVC v142 - VS 2019 C++ x64/x86 build tools (latest)
   - ☑ C++ CMake tools for Windows
   - ☑ C++ ATL for latest v142 build tools (x86 & x64)
   - ☑ C++ MFC for latest v142 build tools (x86 & x64)
   - ☑ Test Adapter for Google Test
   - ☑ Windows 10 SDK (10.0.19041.0)
   - ☑ Git for Windows

4. Click **Modify** to install

---

## Export Configuration from Current Installation

### Using PowerShell script:
```powershell
.\export-vs-config.ps1
```

### Manual export:
```powershell
"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe" export --config .vsconfig
```

---

## Project Dependencies

### Required for Building:
- **MFC (Microsoft Foundation Classes)** - Used throughout the project
- **ATL (Active Template Library)** - Used in some components
- **Windows SDK** - For Windows API
- **CMake** - For ApertureCore build system

### Required for Testing:
- **Google Test** - Unit testing framework
- **Test Adapter for Google Test** - VS integration for tests

---

## Additional Setup

### After Installing Visual Studio:

1. **Configure Git:**
   ```bash
   git config --global core.autocrlf true
   git config --global user.name "Your Name"
   git config --global user.email "your.email@example.com"
   ```

2. **Install vcpkg (if using):**
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

3. **Clone repository:**
   ```bash
   git clone https://github.com/Vovchek/Digit.git
   cd Digit
   ```

4. **Open solution:**
   - Open `Digit.sln` in Visual Studio
   - Visual Studio should detect `.vsconfig` and prompt to install missing components

---

## Verification

After installation, verify setup:

1. **Check compiler:**
   - Open Visual Studio Developer Command Prompt
   - Run: `cl.exe` (should show MSVC compiler version)

2. **Check CMake:**
   - Run: `cmake --version`

3. **Build test:**
   - Open `Digit.sln`
   - Build > Build Solution (Ctrl+Shift+B)
   - Should complete without errors

---

## Troubleshooting

### If .vsconfig is not detected:
1. Close Visual Studio
2. Delete `.vs` folder in solution directory
3. Reopen solution

### If components are missing:
1. Open Visual Studio Installer
2. Click "Modify"
3. Manually verify all required components are installed

### If build fails:
1. Check that Windows SDK version matches project settings
2. Verify MFC/ATL components are installed
3. Check project properties for correct Platform Toolset

---

**Created:** 2024  
**For:** Digit Project Setup  
**Visual Studio:** 2019/2022
