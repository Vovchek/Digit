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

# Extract APERTURE_SOURCES - Match everything until we find the closing parenthesis at start of line
# This handles comments with parentheses inside the set() block
$sourcesMatch = [regex]::Match($cmakeFile, 'set\(APERTURE_SOURCES\s+(.*?)\n\)', [System.Text.RegularExpressions.RegexOptions]::Singleline)

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
    if ($line -match '^\s*#' -or $line -eq '') {
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

# ============================================================================
# NEW: Parse subdirectories for test executables and examples
# ============================================================================

# Function to parse add_executable() from CMakeLists.txt
function Get-ExecutablesFromCMake {
    param([string]$cmakeFilePath, [string]$subdir)
    
    $executables = @()
    
    if (-not (Test-Path $cmakeFilePath)) {
        return $executables
    }
    
    $content = Get-Content $cmakeFilePath -Raw
    
    # Match add_executable(name ...) blocks
    $matches = [regex]::Matches($content, 'add_executable\((\w+)\s+(.*?)\)', [System.Text.RegularExpressions.RegexOptions]::Singleline)
    
    foreach ($match in $matches) {
        $exeName = $match.Groups[1].Value
        $filesBlock = $match.Groups[2].Value
        
        $exeFiles = @()
        foreach ($line in $filesBlock -split "`n") {
            $line = $line.Trim()
            
            # Skip comments and empty lines
            if ($line -match '^\s*#' -or $line -eq '') {
                continue
            }
            
            # Extract file paths (cpp files typically)
            if ($line -match '([\w/]+\.cpp)') {
                $file = $matches[1]
                # Make path relative to ApertureCore root
                $vsPath = "$subdir\$file" -replace '/', '\'
                $exeFiles += $vsPath
            }
        }
        
        if ($exeFiles.Count -gt 0) {
            $executables += @{
                Name = $exeName
                Files = $exeFiles
                Subdir = $subdir
            }
        }
    }
    
    return $executables
}

# Parse tests/CMakeLists.txt
$testExecutables = @()
if (Test-Path "tests\CMakeLists.txt") {
    Write-Host "Parsing tests/CMakeLists.txt..." -ForegroundColor Cyan
    $testExecutables = Get-ExecutablesFromCMake "tests\CMakeLists.txt" "tests"
    
    foreach ($exe in $testExecutables) {
        Write-Host "  Found test: $($exe.Name) ($($exe.Files.Count) files)" -ForegroundColor Yellow
    }
    Write-Host ""
}

# Parse examples/CMakeLists.txt (if it exists)
$exampleExecutables = @()
if (Test-Path "examples\CMakeLists.txt") {
    Write-Host "Parsing examples/CMakeLists.txt..." -ForegroundColor Cyan
    $exampleExecutables = Get-ExecutablesFromCMake "examples\CMakeLists.txt" "examples"
    
    foreach ($exe in $exampleExecutables) {
        Write-Host "  Found example: $($exe.Name) ($($exe.Files.Count) files)" -ForegroundColor Yellow
    }
    Write-Host ""
}

# Collect all test/example source files
$testSourceFiles = @()
foreach ($exe in $testExecutables + $exampleExecutables) {
    $testSourceFiles += $exe.Files
}

# ============================================================================
# End of subdirectory parsing
# ============================================================================

# Auto-discover headers from include directory
$allHeaders = Get-ChildItem -Path "include\aperturecore" -Recurse -Filter "*.h" | 
    ForEach-Object { $_.FullName.Replace((Get-Location).Path + '\', '') }

Write-Host "Auto-discovered headers: $($allHeaders.Count)" -ForegroundColor Yellow
Write-Host ""

# Read .vcxproj
$vcxprojFile = "ApertureCore.vcxproj"
[xml]$vcxproj = Get-Content $vcxprojFile

# Find or create ClCompile and ClInclude ItemGroups
$compileGroup = $vcxproj.Project.ItemGroup | Where-Object { $_.ClCompile -ne $null } | Select-Object -First 1
$includeGroup = $vcxproj.Project.ItemGroup | Where-Object { $_.ClInclude -ne $null } | Select-Object -First 1

# If no ClCompile group exists, find an empty ItemGroup or the one with source files comment
if (-not $compileGroup) {
    # Look for ItemGroup with "Source files" comment
    $compileGroup = $vcxproj.Project.ItemGroup | Where-Object { 
        $_.HasChildNodes -eq $false -or ($_.FirstChild.NodeType -eq [System.Xml.XmlNodeType]::Comment -and $_.FirstChild.Value -match "Source files")
    } | Select-Object -First 1
    
    # If still not found, create a new ItemGroup
    if (-not $compileGroup) {
        $compileGroup = $vcxproj.CreateElement("ItemGroup", $vcxproj.DocumentElement.NamespaceURI)
        $comment = $vcxproj.CreateComment(" Source files ")
        $compileGroup.AppendChild($comment) | Out-Null
        # Insert before the last ItemGroup (which typically has documentation)
        $lastItemGroup = $vcxproj.Project.ItemGroup | Select-Object -Last 1
        $vcxproj.Project.InsertBefore($compileGroup, $lastItemGroup) | Out-Null
    }
}

if (-not $includeGroup) {
    Write-Host "Error: Could not find ClInclude ItemGroup in .vcxproj" -ForegroundColor Red
    exit 1
}

# Get existing files in .vcxproj
$existingCompile = @($compileGroup.ClCompile | ForEach-Object { $_.Include })
$existingInclude = @($includeGroup.ClInclude | ForEach-Object { $_.Include })

# Combine all source files (library + tests + examples)
$allSourceFiles = $sourceFiles + $testSourceFiles

# Compare
$missingInVS_Compile = $allSourceFiles | Where-Object { $_ -notin $existingCompile }
$extraInVS_Compile = $existingCompile | Where-Object { $_ -notin $allSourceFiles }

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
    Write-Host "  ✓ All source files in sync" -ForegroundColor Green
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
    Write-Host "  ✓ All headers in sync" -ForegroundColor Green
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
        Write-Host "  ✓ Saved $vcxprojFile" -ForegroundColor Green
    } else {
        Write-Host "  ✓ No changes needed" -ForegroundColor Green
    }
} else {
    Write-Host "DRY RUN - No changes applied" -ForegroundColor Yellow
    Write-Host "Run without -DryRun to apply changes" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green

# ============================================================================
# Also sync the .vcxproj.filters file
# ============================================================================

Write-Host ""
Write-Host "Syncing .vcxproj.filters..." -ForegroundColor Cyan

$filtersFile = "ApertureCore.vcxproj.filters"
if (Test-Path $filtersFile) {
    [xml]$filters = Get-Content $filtersFile
    
    # Find ItemGroups for ClCompile and ClInclude
    $filtersCompileGroup = $filters.Project.ItemGroup | Where-Object { $_.ClCompile -ne $null } | Select-Object -First 1
    $filtersIncludeGroup = $filters.Project.ItemGroup | Where-Object { $_.ClInclude -ne $null } | Select-Object -First 1
    
    if ($filtersCompileGroup -and $filtersIncludeGroup) {
        # Helper function to determine filter path from file path
        function Get-FilterPath {
            param([string]$filePath)
            
            # Examples:
            # src\geometry\Point.cpp -> Source Files\geometry
            # include\aperturecore\visibility\TypeLimits.h -> Header Files\visibility
            # tests\geometry\PointTest.cpp -> Tests\geometry
            # examples\basic_shapes.cpp -> Examples
            
            if ($filePath -match '^tests\\(\w+)\\') {
                return "Tests\$($matches[1])"
            }
            elseif ($filePath -match '^examples\\') {
                return "Examples"
            }
            elseif ($filePath -match '^src\\(\w+)\\') {
                return "Source Files\$($matches[1])"
            }
            elseif ($filePath -match '^include\\aperturecore\\(\w+)\\') {
                return "Header Files\$($matches[1])"
            }
            
            # Default
            if ($filePath -match '\.cpp$') { return "Source Files" }
            if ($filePath -match '\.h$') { return "Header Files" }
            
            return $null
        }
        
        # Get existing files in filters
        $existingFiltersCompile = @($filtersCompileGroup.ClCompile | ForEach-Object { $_.Include })
        $existingFiltersInclude = @($filtersIncludeGroup.ClInclude | ForEach-Object { $_.Include })
        
        # Find files that need to be added to filters
        $missingFiltersCompile = $allSourceFiles | Where-Object { $_ -notin $existingFiltersCompile }
        $missingFiltersInclude = $allHeaders | Where-Object { $_ -notin $existingFiltersInclude }
        
        # Find files that need to be removed from filters
        $extraFiltersCompile = $existingFiltersCompile | Where-Object { $_ -notin $allSourceFiles }
        $extraFiltersInclude = $existingFiltersInclude | Where-Object { $_ -notin $allHeaders }
        
        if (-not $DryRun) {
            $filtersChanged = $false
            
            # Add missing source files to filters
            foreach ($file in $missingFiltersCompile) {
                $filterPath = Get-FilterPath $file
                if ($filterPath) {
                    $newNode = $filters.CreateElement("ClCompile", $filters.DocumentElement.NamespaceURI)
                    $newNode.SetAttribute("Include", $file)
                    
                    $filterNode = $filters.CreateElement("Filter", $filters.DocumentElement.NamespaceURI)
                    $filterNode.InnerText = $filterPath
                    $newNode.AppendChild($filterNode) | Out-Null
                    
                    $filtersCompileGroup.AppendChild($newNode) | Out-Null
                    Write-Host "  Filters: Added $file -> $filterPath" -ForegroundColor Green
                    $filtersChanged = $true
                }
            }
            
            # Add missing headers to filters
            foreach ($file in $missingFiltersInclude) {
                $filterPath = Get-FilterPath $file
                if ($filterPath) {
                    $newNode = $filters.CreateElement("ClInclude", $filters.DocumentElement.NamespaceURI)
                    $newNode.SetAttribute("Include", $file)
                    
                    $filterNode = $filters.CreateElement("Filter", $filters.DocumentElement.NamespaceURI)
                    $filterNode.InnerText = $filterPath
                    $newNode.AppendChild($filterNode) | Out-Null
                    
                    $filtersIncludeGroup.AppendChild($newNode) | Out-Null
                    Write-Host "  Filters: Added $file -> $filterPath" -ForegroundColor Green
                    $filtersChanged = $true
                }
            }
            
            # Remove extra files from filters
            foreach ($file in $extraFiltersCompile) {
                $node = $filtersCompileGroup.ClCompile | Where-Object { $_.Include -eq $file }
                if ($node) {
                    $filtersCompileGroup.RemoveChild($node) | Out-Null
                    Write-Host "  Filters: Removed $file" -ForegroundColor Yellow
                    $filtersChanged = $true
                }
            }
            
            foreach ($file in $extraFiltersInclude) {
                $node = $filtersIncludeGroup.ClInclude | Where-Object { $_.Include -eq $file }
                if ($node) {
                    $filtersIncludeGroup.RemoveChild($node) | Out-Null
                    Write-Host "  Filters: Removed $file" -ForegroundColor Yellow
                    $filtersChanged = $true
                }
            }
            
            if ($filtersChanged) {
                # Backup
                Copy-Item $filtersFile "$filtersFile.backup"
                Write-Host ""
                Write-Host "  Filters backup: $filtersFile.backup" -ForegroundColor Cyan
                
                # Save
                $filters.Save((Resolve-Path $filtersFile))
                Write-Host "  ✓ Saved $filtersFile" -ForegroundColor Green
            } else {
                Write-Host "  ✓ Filters already in sync" -ForegroundColor Green
            }
        } else {
            if ($missingFiltersCompile.Count -gt 0 -or $missingFiltersInclude.Count -gt 0) {
                Write-Host "  Would add $($missingFiltersCompile.Count) source files and $($missingFiltersInclude.Count) headers to filters" -ForegroundColor Yellow
            } else {
                Write-Host "  Filters already in sync" -ForegroundColor Green
            }
        }
    } else {
        Write-Host "  Warning: Could not find ItemGroups in .vcxproj.filters" -ForegroundColor Yellow
    }
} else {
    Write-Host "  Warning: $filtersFile not found" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
