# Visual Studio Encoding Configuration Script
# Forces Visual Studio to respect UTF-8 encoding for markdown files

param(
    [switch]$ShowCurrent
)

Write-Host "Visual Studio Encoding Configuration" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Registry paths for Visual Studio settings (VS 2019/2022)
$vsVersions = @(
    "17.0",  # VS 2022
    "16.0",  # VS 2019
    "15.0"   # VS 2017
)

$settingsFound = $false

foreach ($version in $vsVersions) {
    $regPath = "HKCU:\Software\Microsoft\VisualStudio\${version}_Config\Text Editor"
    
    if (Test-Path $regPath) {
        $settingsFound = $true
        Write-Host "Found Visual Studio $version settings" -ForegroundColor Green
        
        # Check current encoding settings
        $encodingPath = "$regPath\File Extension\.md"
        
        if ($ShowCurrent) {
            Write-Host ""
            Write-Host "Current settings for .md files:" -ForegroundColor Yellow
            if (Test-Path $encodingPath) {
                Get-ItemProperty -Path $encodingPath | Format-List
            } else {
                Write-Host "  No specific settings found for .md files" -ForegroundColor Gray
            }
        } else {
            Write-Host "  Registry path: $encodingPath" -ForegroundColor Gray
        }
    }
}

if (-not $settingsFound) {
    Write-Host "No Visual Studio installations found in registry" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Manual Configuration Steps:" -ForegroundColor Cyan
Write-Host "===========================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. Open Visual Studio" -ForegroundColor Yellow
Write-Host "2. Tools → Options" -ForegroundColor Yellow
Write-Host "3. Environment → Documents" -ForegroundColor Yellow
Write-Host "4. CHECK: 'Save documents as Unicode (UTF-8 with signature) - Codepage 65001'" -ForegroundColor Yellow
Write-Host "5. Click OK" -ForegroundColor Yellow
Write-Host ""
Write-Host "6. Close ALL open .md files in Visual Studio" -ForegroundColor Yellow
Write-Host "7. Run: .\fix-encoding.ps1" -ForegroundColor Cyan
Write-Host "8. Reopen .md files in Visual Studio" -ForegroundColor Yellow
Write-Host ""
Write-Host "If Unicode symbols STILL show incorrectly:" -ForegroundColor Red
Write-Host "==========================================" -ForegroundColor Red
Write-Host ""
Write-Host "Visual Studio has a known bug with UTF-8 BOM on non-English systems." -ForegroundColor Yellow
Write-Host ""
Write-Host "WORKAROUND: Replace Unicode symbols with ASCII equivalents" -ForegroundColor Cyan
Write-Host "  Run: .\fix-encoding.ps1 -StripUnicode" -ForegroundColor White
Write-Host ""
Write-Host "This converts:" -ForegroundColor Gray
Write-Host "  ✅ → [OK]" -ForegroundColor Gray
Write-Host "  ❌ → [X]" -ForegroundColor Gray
Write-Host "  ⚠️ → [!]" -ForegroundColor Gray
Write-Host "  🔄 → [~]" -ForegroundColor Gray
Write-Host "  etc." -ForegroundColor Gray
Write-Host ""
Write-Host "Alternative: Use VS Code or another editor for .md files" -ForegroundColor Cyan
Write-Host "  VS Code respects UTF-8 BOM correctly" -ForegroundColor Gray
Write-Host ""
Write-Host "Done!" -ForegroundColor Green
