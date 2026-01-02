# Sync CMakeLists.txt to Visual Studio project
# Run this after modifying CMakeLists.txt to update .vcxproj

param(
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

Write-Host "ApertureCore Build System Sync" -ForegroundColor Cyan
Write-Host "===============================" -ForegroundColor Cyan
Write-Host ""

# Parse CMakeLists.txt
$cmakeFile = Get-Content "CMakeLists.txt" -Raw

# Extract APERTURE_SOURCES
$sourcesMatch = [regex]::Match($cmakeFile, 'set\(APERTURE_SOURCES\s+(.*?)\)', [System.Text.RegularExpressions.RegexOptions]::Singleline)

if (-not $sourcesMatch.Success) {
    Write-Host "Error: Could not find APERTURE_SOURCES in CMakeLists.txt" -ForegroundColor Red
    exit 1
}

$sourcesBlock = $sourcesMatch.Groups[1].Value

# Extract file paths (skip comments and TODO lines)
$sourceFiles = @()
$headerFiles = @()

foreach ($line in $sourcesBlock -split "`n") {
    $line = $line.Trim()
    
    # Skip comments, empty lines, and TODO lines
    if ($line -match '^\s*#' -or $line -eq '' -or $line -match '^\s*\)') {
        continue
    }
    
    # Extract file path - look for src/... patterns
    if ($line -match 'src/([\w/]+\.(?:cpp|h))') {
        $file = "src/" + $matches[1]
        
        # Convert forward slashes to backslashes for VS
        $vsPath = $file -replace '/', '\'
        
        if ($file -match '\.cpp$') {
            $sourceFiles += $vsPath
        } elseif ($file -match '\.h$') {
            $headerFiles += $vsPath
        }
    }
}

Write-Host "Found in CMakeLists.txt:" -ForegroundColor Green
Write-Host "  Source files: $($sourceFiles.Count)" -ForegroundColor Yellow
Write-Host "  Header files: $($headerFiles.Count)" -ForegroundColor Yellow
Write-Host ""

# Auto-discover headers from include directory
$allHeaders = Get-ChildItem -Path "include\aperturecore" -Recurse -Filter "*.h" | 
    ForEach-Object { $_.FullName.Replace((Get-Location).Path + '\', '') }

Write-Host "Auto-discovered headers: $($allHeaders.Count)" -ForegroundColor Yellow
Write-Host ""

# Read .vcxproj
$vcxprojFile = "ApertureCore.vcxproj"
[xml]$vcxproj = Get-Content $vcxprojFile

# Find ClCompile ItemGroup
$compileGroup = $vcxproj.Project.ItemGroup | Where-Object { $_.ClCompile -ne $null } | Select-Object -First 1
$includeGroup = $vcxproj.Project.ItemGroup | Where-Object { $_.ClInclude -ne $null } | Select-Object -First 1

if (-not $compileGroup) {
    Write-Host "Error: Could not find ClCompile ItemGroup in .vcxproj" -ForegroundColor Red
    exit 1
}

# Get existing files in .vcxproj
$existingCompile = @($compileGroup.ClCompile | ForEach-Object { $_.Include })
$existingInclude = @($includeGroup.ClInclude | ForEach-Object { $_.Include })

# Compare
$missingInVS_Compile = $sourceFiles | Where-Object { $_ -notin $existingCompile }
$extraInVS_Compile = $existingCompile | Where-Object { $_ -notin $sourceFiles }

$missingInVS_Include = $allHeaders | Where-Object { $_ -notin $existingInclude }
$extraInVS_Include = $existingInclude | Where-Object { $_ -notin $allHeaders }

# Report
Write-Host "Synchronization Analysis:" -ForegroundColor Cyan
Write-Host ""

if ($missingInVS_Compile.Count -gt 0) {
    Write-Host "  Missing in .vcxproj (source files):" -ForegroundColor Yellow
    foreach ($file in $missingInVS_Compile) {
        Write-Host "    + $file" -ForegroundColor Green
    }
} else {
    Write-Host "  ? All source files in sync" -ForegroundColor Green
}

if ($extraInVS_Compile.Count -gt 0) {
    Write-Host "  Extra in .vcxproj (source files):" -ForegroundColor Yellow
    foreach ($file in $extraInVS_Compile) {
        Write-Host "    - $file" -ForegroundColor Red
    }
}

Write-Host ""

if ($missingInVS_Include.Count -gt 0) {
    Write-Host "  Missing in .vcxproj (headers):" -ForegroundColor Yellow
    foreach ($file in $missingInVS_Include) {
        Write-Host "    + $file" -ForegroundColor Green
    }
} else {
    Write-Host "  ? All headers in sync" -ForegroundColor Green
}

if ($extraInVS_Include.Count -gt 0) {
    Write-Host "  Extra in .vcxproj (headers):" -ForegroundColor Yellow
    foreach ($file in $extraInVS_Include) {
        Write-Host "    - $file" -ForegroundColor Red
    }
}

Write-Host ""

# Apply changes
if (-not $DryRun) {
    $changed = $false
    
    # Add missing source files
    foreach ($file in $missingInVS_Compile) {
        $newNode = $vcxproj.CreateElement("ClCompile", $vcxproj.DocumentElement.NamespaceURI)
        $newNode.SetAttribute("Include", $file)
        $compileGroup.AppendChild($newNode) | Out-Null
        Write-Host "  Added: $file" -ForegroundColor Green
        $changed = $true
    }
    
    # Add missing headers
    foreach ($file in $missingInVS_Include) {
        $newNode = $vcxproj.CreateElement("ClInclude", $vcxproj.DocumentElement.NamespaceURI)
        $newNode.SetAttribute("Include", $file)
        $includeGroup.AppendChild($newNode) | Out-Null
        Write-Host "  Added: $file" -ForegroundColor Green
        $changed = $true
    }
    
    # Remove extra files
    foreach ($file in $extraInVS_Compile) {
        $node = $compileGroup.ClCompile | Where-Object { $_.Include -eq $file }
        if ($node) {
            $compileGroup.RemoveChild($node) | Out-Null
            Write-Host "  Removed: $file" -ForegroundColor Yellow
            $changed = $true
        }
    }
    
    foreach ($file in $extraInVS_Include) {
        $node = $includeGroup.ClInclude | Where-Object { $_.Include -eq $file }
        if ($node) {
            $includeGroup.RemoveChild($node) | Out-Null
            Write-Host "  Removed: $file" -ForegroundColor Yellow
            $changed = $true
        }
    }
    
    if ($changed) {
        # Backup
        Copy-Item $vcxprojFile "$vcxprojFile.backup"
        Write-Host ""
        Write-Host "  Backup created: $vcxprojFile.backup" -ForegroundColor Cyan
        
        # Save
        $vcxproj.Save((Resolve-Path $vcxprojFile))
        Write-Host "  ? Saved $vcxprojFile" -ForegroundColor Green
    } else {
        Write-Host "  ? No changes needed" -ForegroundColor Green
    }
} else {
    Write-Host "DRY RUN - No changes applied" -ForegroundColor Yellow
    Write-Host "Run without -DryRun to apply changes" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
