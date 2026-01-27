# ============================================================
# CFringe → CFringeSegment Refactoring Script
# ============================================================
# Purpose: Rename CFringe class to CFringeSegment (segment-primary model)
# Date: 2026-01-26
# Model: Segment-primary architecture
# ============================================================

param(
    [switch]$DryRun = $false,
    [switch]$SkipBuild = $false
)

$ErrorActionPreference = "Stop"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " CFringe → CFringeSegment Refactoring Script" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

if ($DryRun) {
    Write-Host "[DRY RUN MODE] No files will be modified" -ForegroundColor Yellow
    Write-Host ""
}

# ============================================================
# 1. Backup Current State
# ============================================================

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$backupDir = ".\Backups\Refactor_$timestamp"

if (-not $DryRun) {
    Write-Host "[BACKUP] Creating backup at: $backupDir" -ForegroundColor Green
    New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
    
    # Backup files to be renamed
    $filesToBackup = @(
        "DigitMode\CFringe.h",
        "DigitMode\CFringe.cpp",
        "Tests\DigitModeTests\CFringeTest.cpp",
        "Tests\DigitModeTests\CFringeFileIOTest.cpp",
        "Digit.vcxproj",
        "Digit.vcxproj.filters"
    )
    
    foreach ($file in $filesToBackup) {
        if (Test-Path $file) {
            $destDir = Join-Path $backupDir (Split-Path $file -Parent)
            New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            Copy-Item $file -Destination (Join-Path $destDir (Split-Path $file -Leaf))
            Write-Host "  ✓ Backed up: $file" -ForegroundColor Gray
        }
    }
}

# ============================================================
# 2. File Renaming (Physical Files)
# ============================================================

Write-Host ""
Write-Host "[RENAME] Renaming files..." -ForegroundColor Green

$fileRenames = @(
    @{ Old = "DigitMode\CFringe.h";                      New = "DigitMode\CFringeSegment.h" },
    @{ Old = "DigitMode\CFringe.cpp";                    New = "DigitMode\CFringeSegment.cpp" },
    @{ Old = "Tests\DigitModeTests\CFringeTest.cpp";     New = "Tests\DigitModeTests\CFringeSegmentTest.cpp" },
    @{ Old = "Tests\DigitModeTests\CFringeFileIOTest.cpp"; New = "Tests\DigitModeTests\CFringeSegmentFileIOTest.cpp" }
)

foreach ($rename in $fileRenames) {
    if (Test-Path $rename.Old) {
        if ($DryRun) {
            Write-Host "  [DRY RUN] Would rename: $($rename.Old) → $($rename.New)" -ForegroundColor Yellow
        } else {
            Rename-Item -Path $rename.Old -NewName (Split-Path $rename.New -Leaf) -Force
            Write-Host "  ✓ Renamed: $($rename.Old) → $($rename.New)" -ForegroundColor Green
        }
    } else {
        Write-Host "  ⚠ File not found: $($rename.Old)" -ForegroundColor Yellow
    }
}

# ============================================================
# 3. Content Replacement (In-File Class Names)
# ============================================================

Write-Host ""
Write-Host "[CONTENT] Updating class names in files..." -ForegroundColor Green

function Replace-InFile {
    param(
        [string]$FilePath,
        [hashtable]$Replacements
    )
    
    if (-not (Test-Path $FilePath)) {
        Write-Host "  ⚠ File not found: $FilePath" -ForegroundColor Yellow
        return
    }
    
    $content = Get-Content $FilePath -Raw -Encoding UTF8
    $originalContent = $content
    
    foreach ($key in $Replacements.Keys) {
        $content = $content -replace $key, $Replacements[$key]
    }
    
    if ($content -ne $originalContent) {
        if ($DryRun) {
            Write-Host "  [DRY RUN] Would update: $FilePath" -ForegroundColor Yellow
        } else {
            Set-Content -Path $FilePath -Value $content -Encoding UTF8 -NoNewline
            Write-Host "  ✓ Updated: $FilePath" -ForegroundColor Green
        }
    } else {
        Write-Host "  ○ No changes needed: $FilePath" -ForegroundColor Gray
    }
}

# Replacements (regex patterns)
$classReplacements = @{
    '\bCFringe\b(?!Segment)' = 'CFringeSegment'  # CFringe → CFringeSegment (but not CFringeSegment → CFringeSegmentSegment)
}

# Files to update (after renaming)
$filesToUpdate = @(
    "DigitMode\CFringeSegment.h",
    "DigitMode\CFringeSegment.cpp",
    "Tests\DigitModeTests\CFringeSegmentTest.cpp",
    "Tests\DigitModeTests\CFringeSegmentFileIOTest.cpp",
    "DigitMode\DigitInfo.h",
    "DigitMode\DigitInfo.cpp"
)

foreach ($file in $filesToUpdate) {
    Replace-InFile -FilePath $file -Replacements $classReplacements
}

# ============================================================
# 4. Update Project Files (.vcxproj, .vcxproj.filters)
# ============================================================

Write-Host ""
Write-Host "[PROJECT] Updating Visual Studio project files..." -ForegroundColor Green

$projectReplacements = @{
    'DigitMode\\CFringe\.h'                      = 'DigitMode\CFringeSegment.h'
    'DigitMode\\CFringe\.cpp'                    = 'DigitMode\CFringeSegment.cpp'
    'Tests\\DigitModeTests\\CFringeTest\.cpp'    = 'Tests\DigitModeTests\CFringeSegmentTest.cpp'
    'Tests\\DigitModeTests\\CFringeFileIOTest\.cpp' = 'Tests\DigitModeTests\CFringeSegmentFileIOTest.cpp'
}

Replace-InFile -FilePath "Digit.vcxproj" -Replacements $projectReplacements
Replace-InFile -FilePath "Digit.vcxproj.filters" -Replacements $projectReplacements

# ============================================================
# 5. Search for Remaining References
# ============================================================

Write-Host ""
Write-Host "[VERIFY] Searching for remaining 'CFringe' references..." -ForegroundColor Green

$searchPattern = '\bCFringe\b(?!Segment)'
$excludeDirs = @('.git', 'Backups', 'bin', 'obj', 'x64', 'Debug', 'Release', '.vs')
$excludeExts = @('.exe', '.dll', '.obj', '.pdb', '.ilk', '.suo', '.user', '.aps')

$foundRefs = @()

Get-ChildItem -Path . -Recurse -File | Where-Object {
    $path = $_.FullName
    $excluded = $false
    
    foreach ($dir in $excludeDirs) {
        if ($path -like "*\$dir\*") {
            $excluded = $true
            break
        }
    }
    
    foreach ($ext in $excludeExts) {
        if ($_.Extension -eq $ext) {
            $excluded = $true
            break
        }
    }
    
    -not $excluded -and ($_.Extension -match '\.(h|cpp|c|md|txt|vcxproj)$')
} | ForEach-Object {
    $content = Get-Content $_.FullName -Raw -ErrorAction SilentlyContinue
    if ($content -match $searchPattern) {
        $relativePath = $_.FullName.Replace((Get-Location).Path + '\', '')
        $foundRefs += $relativePath
        Write-Host "  ⚠ Found 'CFringe' in: $relativePath" -ForegroundColor Yellow
    }
}

if ($foundRefs.Count -eq 0) {
    Write-Host "  ✓ No remaining 'CFringe' references found!" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "  Found $($foundRefs.Count) files with remaining references." -ForegroundColor Yellow
    Write-Host "  Please review these manually." -ForegroundColor Yellow
}

# ============================================================
# 6. Build Verification
# ============================================================

if (-not $SkipBuild -and -not $DryRun) {
    Write-Host ""
    Write-Host "[BUILD] Attempting build to verify health..." -ForegroundColor Green
    
    # Try to find MSBuild
    $msbuildPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    )
    
    $msbuild = $null
    foreach ($path in $msbuildPaths) {
        if (Test-Path $path) {
            $msbuild = $path
            break
        }
    }
    
    if ($msbuild) {
        Write-Host "  Using MSBuild: $msbuild" -ForegroundColor Gray
        & $msbuild "Digit.sln" /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /m /v:minimal
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host ""
            Write-Host "  ✓ BUILD SUCCESSFUL!" -ForegroundColor Green
        } else {
            Write-Host ""
            Write-Host "  ✗ BUILD FAILED (Exit code: $LASTEXITCODE)" -ForegroundColor Red
            Write-Host "  Please review errors above." -ForegroundColor Yellow
        }
    } else {
        Write-Host "  ⚠ MSBuild not found. Skipping build." -ForegroundColor Yellow
        Write-Host "  Please build manually in Visual Studio." -ForegroundColor Yellow
    }
}

# ============================================================
# Summary
# ============================================================

Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Refactoring Complete!" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Files renamed:" -ForegroundColor White
foreach ($rename in $fileRenames) {
    Write-Host "  • $($rename.Old) → $($rename.New)" -ForegroundColor Gray
}
Write-Host ""

if ($DryRun) {
    Write-Host "[DRY RUN] No changes were made. Run without -DryRun to apply." -ForegroundColor Yellow
} else {
    Write-Host "Next steps:" -ForegroundColor White
    Write-Host "  1. Review changes in Git" -ForegroundColor Gray
    Write-Host "  2. Run tests to verify functionality" -ForegroundColor Gray
    Write-Host "  3. Commit with message: 'Refactor: Rename CFringe → CFringeSegment (segment-primary model)'" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Backup created at: $backupDir" -ForegroundColor Green
}

Write-Host ""
