#!/usr/bin/env powershell
# Build script for ApertureCore library
# Usage: .\build.ps1 [Debug|Release] [--clean] [--test] [--install] [--docs]

param(
    [string]$BuildType = "Debug",
    [switch]$Clean,
    [switch]$Test,
    [switch]$Install,
    [switch]$Docs,
    [string]$InstallPrefix = ""
)

$ErrorActionPreference = "Stop"

# Colors for output
function Write-Info { Write-Host $args -ForegroundColor Cyan }
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Error { Write-Host $args -ForegroundColor Red }

Write-Info "ApertureCore Build Script"
Write-Info "========================="
Write-Info ""

# Validate build type
if ($BuildType -notin @("Debug", "Release")) {
    Write-Error "Invalid build type: $BuildType"
    Write-Error "Valid options: Debug, Release"
    exit 1
}

# Project root
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

Write-Info "Project Root: $ProjectRoot"
Write-Info "Build Type:   $BuildType"
Write-Info ""

# Clean if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Info "Cleaning build directory..."
    Remove-Item -Recurse -Force $BuildDir
    Write-Success "✓ Clean complete"
    Write-Info ""
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Info "Creating build directory..."
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    Write-Success "✓ Build directory created"
}

# Configure CMake
Write-Info "Configuring CMake..."
Push-Location $BuildDir
try {
    $CMakeArgs = @(
        ".."
        "-DCMAKE_BUILD_TYPE=$BuildType"
        "-DAPERTURE_BUILD_TESTS=ON"
        "-DAPERTURE_BUILD_LEGACY=ON"
        "-DAPERTURE_BUILD_DOCS=ON"
    )
    
    if ($InstallPrefix) {
        $CMakeArgs += "-DCMAKE_INSTALL_PREFIX=$InstallPrefix"
    }
    
    & cmake @CMakeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    Write-Success "✓ CMake configuration complete"
    Write-Info ""
} catch {
    Write-Error "Configuration failed: $_"
    Pop-Location
    exit 1
}

# Build
Write-Info "Building ApertureCore ($BuildType)..."
try {
    & cmake --build . --config $BuildType
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    Write-Success "✓ Build complete"
    Write-Info ""
} catch {
    Write-Error "Build failed: $_"
    Pop-Location
    exit 1
}

# Build documentation if requested
if ($Docs) {
    Write-Info "Building documentation..."
    try {
        & cmake --build . --target docs
        if ($LASTEXITCODE -ne 0) {
            throw "Documentation build failed"
        }
        
        # FIX: Documentation is in PROJECT_ROOT/docs, not build/docs
        $DocsPath = Join-Path $ProjectRoot "docs\html\index.html"
        if (Test-Path $DocsPath) {
            Write-Success "✓ Documentation built successfully"
            Write-Info "  Documentation: $DocsPath"
        } else {
            Write-Warning "Documentation built but index.html not found at expected location"
            Write-Warning "  Expected: $DocsPath"
        }
        Write-Info ""
    } catch {
        Write-Error "Documentation build failed: $_"
        Pop-Location
        exit 1
    }
}

# Test if requested
if ($Test) {
    Write-Info "Running tests..."
    try {
        & ctest --build-config $BuildType --output-on-failure
        if ($LASTEXITCODE -ne 0) {
            throw "Tests failed"
        }
        Write-Success "✓ All tests passed"
        Write-Info ""
    } catch {
        Write-Error "Tests failed: $_"
        Pop-Location
        exit 1
    }
}

# Install if requested
if ($Install) {
    Write-Info "Installing ApertureCore..."
    try {
        & cmake --install . --config $BuildType
        if ($LASTEXITCODE -ne 0) {
            throw "Installation failed"
        }
        Write-Success "✓ Installation complete"
        Write-Info ""
    } catch {
        Write-Error "Installation failed: $_"
        Pop-Location
        exit 1
    }
}

Pop-Location

# Summary
Write-Info "Build Summary"
Write-Info "============="
Write-Success "✓ Configuration: $BuildType"
Write-Success "✓ Build:         Success"
if ($Docs) {
    Write-Success "✓ Docs:          Generated"
}
if ($Test) {
    Write-Success "✓ Tests:         Passed"
}
if ($Install) {
    Write-Success "✓ Install:       Complete"
}

Write-Info ""
Write-Info "Build artifacts in: $BuildDir"
Write-Info ""
Write-Success "Build script completed successfully! 🎉"
