# Add ApertureCore to Digit.sln
# Run from Digit solution directory

$ErrorActionPreference = "Stop"

$solutionFile = "Digit.sln"
$projectPath = "ApertureCore\ApertureCore.vcxproj"
$projectGuid = "{APERTURE-" + (New-Guid).ToString().ToUpper() + "}"

Write-Host "Adding ApertureCore to Digit.sln..." -ForegroundColor Cyan

# Read solution file
$solutionContent = Get-Content $solutionFile -Raw

# Check if already added
if ($solutionContent -match "ApertureCore") {
    Write-Host "ApertureCore is already in the solution!" -ForegroundColor Yellow
    exit 0
}

# Find the last Project entry
$lastProjectEnd = $solutionContent.LastIndexOf("EndProject")

if ($lastProjectEnd -eq -1) {
    Write-Host "Error: Could not find project entries in solution file" -ForegroundColor Red
    exit 1
}

# Create new project entry
$newProjectEntry = @"

Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "ApertureCore", "$projectPath", "$projectGuid"
EndProject
"@

# Insert after last project
$insertPosition = $lastProjectEnd + "EndProject".Length
$newContent = $solutionContent.Insert($insertPosition, $newProjectEntry)

# Backup original
Copy-Item $solutionFile "$solutionFile.backup"
Write-Host "Created backup: $solutionFile.backup" -ForegroundColor Green

# Write new solution
Set-Content -Path $solutionFile -Value $newContent -NoNewline

Write-Host "? Successfully added ApertureCore to solution!" -ForegroundColor Green
Write-Host ""
Write-Host "Project GUID: $projectGuid" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Open Digit.sln in Visual Studio"
Write-Host "2. Right-click solution ? Properties ? Configuration Properties"
Write-Host "3. Set up project dependencies if needed"
Write-Host "4. Build solution"
