# Quick Test Runner for ApertureCore
# Builds and runs all tests with minimal output

param(
    [string]$Filter = "*",
    [string]$Config = "Debug",
    [switch]$Verbose,
    [switch]$NoBuild
)

$ErrorActionPreference = "Stop"

Write-Host "ApertureCore Test Runner" -ForegroundColor Cyan
Write-Host "========================" -ForegroundColor Cyan
Write-Host ""

$buildDir = "build"

# Build if requested
if (-not $NoBuild) {
    Write-Host "Building..." -ForegroundColor Yellow
    
    # Configure if build dir doesn't exist
    if (-not (Test-Path $buildDir)) {
        cmake -B $buildDir -S . -DAPERTURE_BUILD_TESTS=ON
        if ($LASTEXITCODE -ne 0) {
            Write-Host "CMake configure failed!" -ForegroundColor Red
            exit 1
        }
    }
    
    # Build
    cmake --build $buildDir --config $Config
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "✓ Build successful" -ForegroundColor Green
    Write-Host ""
}

# Run tests
Write-Host "Running tests..." -ForegroundColor Yellow
Write-Host "  Filter: $Filter"
Write-Host "  Config: $Config"
Write-Host ""

Push-Location $buildDir

try {
    $ctestArgs = @(
        "-C", $Config,
        "-R", $Filter,
        "--output-on-failure"
    )
    
    if ($Verbose) {
        $ctestArgs += "--verbose"
    }
    
    ctest @ctestArgs
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "✓ All tests passed!" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "✗ Some tests failed" -ForegroundColor Red
        exit 1
    }
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
