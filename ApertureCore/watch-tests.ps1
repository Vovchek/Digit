# Watch mode for tests - runs tests on file changes
# Useful during development

param(
    [string]$Filter = "*",
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

Write-Host "ApertureCore Test Watch Mode" -ForegroundColor Cyan
Write-Host "=============================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Watching for changes in:" -ForegroundColor Yellow
Write-Host "  - include/aperturecore/**/*.h" -ForegroundColor Gray
Write-Host "  - src/**/*.cpp" -ForegroundColor Gray
Write-Host "  - tests/**/*.cpp" -ForegroundColor Gray
Write-Host ""
Write-Host "Press Ctrl+C to exit" -ForegroundColor Gray
Write-Host ""

# Initial build and test run
Write-Host "Initial build and test run..." -ForegroundColor Cyan
.\run-tests.ps1 -Filter $Filter -Config $Config

# Set up file watcher
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = (Get-Location).Path
$watcher.Filter = "*.*"
$watcher.IncludeSubdirectories = $true
$watcher.NotifyFilter = [System.IO.NotifyFilters]::LastWrite -bor [System.IO.NotifyFilters]::FileName

# Track last change time to debounce
$script:lastRun = Get-Date

$action = {
    $path = $Event.SourceEventArgs.FullPath
    $changeType = $Event.SourceEventArgs.ChangeType
    
    # Only watch relevant files
    if ($path -notmatch '\.(cpp|h|hpp)$') {
        return
    }
    
    # Skip build directory
    if ($path -match '\\build\\' -or $path -match '\\out\\') {
        return
    }
    
    # Debounce - wait at least 2 seconds between runs
    $now = Get-Date
    $elapsed = ($now - $script:lastRun).TotalSeconds
    if ($elapsed -lt 2) {
        return
    }
    
    $script:lastRun = $now
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Change detected: $changeType - $path" -ForegroundColor Yellow
    Write-Host "Rebuilding and running tests..." -ForegroundColor Cyan
    Write-Host ""
    
    # Run tests
    & "$PSScriptRoot\run-tests.ps1" -Filter $Filter -Config $Config
}

# Register event handlers
$handlers = @()
$handlers += Register-ObjectEvent -InputObject $watcher -EventName Changed -Action $action
$handlers += Register-ObjectEvent -InputObject $watcher -EventName Created -Action $action
$handlers += Register-ObjectEvent -InputObject $watcher -EventName Renamed -Action $action

# Start watching
$watcher.EnableRaisingEvents = $true

try {
    # Wait forever (until Ctrl+C)
    while ($true) {
        Start-Sleep -Seconds 1
    }
} finally {
    # Cleanup
    Write-Host ""
    Write-Host "Stopping watcher..." -ForegroundColor Yellow
    $watcher.EnableRaisingEvents = $false
    $handlers | ForEach-Object { Unregister-Event -SourceIdentifier $_.Name }
    $watcher.Dispose()
    Write-Host "Done!" -ForegroundColor Green
}
