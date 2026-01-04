# Run a single test executable with GTest filter
# Useful for debugging specific test cases

param(
    [Parameter(Mandatory=$true)]
    [ValidateSet("geometry", "visibility", "integration")]
    [string]$Suite,
    
    [string]$Filter = "*",
    [string]$Config = "Debug",
    [switch]$Verbose,
    [switch]$List,
    [switch]$Shuffle,
    [int]$Repeat = 1
)

$ErrorActionPreference = "Stop"

Write-Host "ApertureCore Single Test Runner" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# Determine test executable
$exeName = switch ($Suite) {
    "geometry"    { "geometry_tests" }
    "visibility"  { "visibility_tests" }
    "integration" { "integration_tests" }
}

$exePath = "build\$Config\$exeName.exe"

# Check if executable exists
if (-not (Test-Path $exePath)) {
    Write-Host "Error: Test executable not found: $exePath" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please build tests first:" -ForegroundColor Yellow
    Write-Host "  cd ApertureCore" -ForegroundColor Gray
    Write-Host "  cmake -B build -S . -DAPERTURE_BUILD_TESTS=ON" -ForegroundColor Gray
    Write-Host "  cmake --build build --config $Config" -ForegroundColor Gray
    exit 1
}

# Build GTest arguments
$gtestArgs = @()

if ($List) {
    $gtestArgs += "--gtest_list_tests"
}

if ($Filter -ne "*") {
    $gtestArgs += "--gtest_filter=$Filter"
}

if ($Verbose) {
    $gtestArgs += "--gtest_print_time=1"
}

if ($Shuffle) {
    $gtestArgs += "--gtest_shuffle"
}

if ($Repeat -gt 1) {
    $gtestArgs += "--gtest_repeat=$Repeat"
}

# Run test
Write-Host "Test Suite: $Suite" -ForegroundColor Yellow
Write-Host "Executable: $exePath" -ForegroundColor Gray
Write-Host "Filter: $Filter" -ForegroundColor Gray
Write-Host ""

& $exePath @gtestArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✓ Tests passed!" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "✗ Tests failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Examples:" -ForegroundColor Cyan
Write-Host "  # Run all geometry tests" -ForegroundColor Gray
Write-Host "  .\run-single-test.ps1 -Suite geometry" -ForegroundColor Gray
Write-Host ""
Write-Host "  # Run specific test case" -ForegroundColor Gray
Write-Host "  .\run-single-test.ps1 -Suite geometry -Filter 'PolygonTest.Area'" -ForegroundColor Gray
Write-Host ""
Write-Host "  # List all tests without running" -ForegroundColor Gray
Write-Host "  .\run-single-test.ps1 -Suite visibility -List" -ForegroundColor Gray
Write-Host ""
Write-Host "  # Run with shuffle and repeat" -ForegroundColor Gray
Write-Host "  .\run-single-test.ps1 -Suite geometry -Shuffle -Repeat 10" -ForegroundColor Gray
