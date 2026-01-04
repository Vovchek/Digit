# Fix UTF-8 Encoding for Markdown Files
# Enhanced version with Visual Studio encoding workaround
# Converts all .md files to UTF-8 with BOM for proper display in Visual Studio

param(
    [switch]$DryRun,
    [switch]$Verbose,
    [switch]$StripUnicode  # Replace Unicode symbols with ASCII equivalents
)

$ErrorActionPreference = "Stop"

Write-Host "UTF-8 Encoding Fix for Markdown Files (Enhanced)" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

if ($StripUnicode) {
    Write-Host "WARNING: Running in Unicode Strip mode - will replace symbols with ASCII" -ForegroundColor Yellow
    Write-Host ""
}

# Unicode to ASCII mapping for Visual Studio compatibility
$unicodeReplacements = @{
    '✅' = '[OK]'
    '✓' = '[x]'
    '❌' = '[X]'
    '⚠️' = '[!]'
    '⚠' = '[!]'
    '🔄' = '[~]'
    '📝' = '[ ]'
    '📍' = '[*]'
    '🔧' = '[+]'
    '🎉' = '(*)'
    '→' = '->'
    '←' = '<-'
    '↔' = '<->'
    '⇒' = '=>'
    '⇐' = '<='
    '⇔' = '<=>'
    '•' = '*'
    '◆' = '-'
    '■' = '#'
    '□' = 'o'
    '▪' = '-'
    '▫' = 'o'
    '†' = '+'
    '‡' = '++'
    '§' = 'S'
    '¶' = 'P'
    '©' = '(c)'
    '®' = '(R)'
    '™' = '(TM)'
    '…' = '...'
    '–' = '-'
    '—' = '--'
    ''' = "'"
    ''' = "'"
    '"' = '"'
    '"' = '"'
    '«' = '<<'
    '»' = '>>'
}

# Get all markdown files
$mdFiles = Get-ChildItem -Path . -Filter "*.md" -Recurse | Where-Object { 
    $_.FullName -notmatch '\\out\\' -and 
    $_.FullName -notmatch '\\build\\' -and
    $_.FullName -notmatch '\\docs\\html\\' 
}

Write-Host "Found $($mdFiles.Count) markdown files" -ForegroundColor Green
Write-Host ""

$converted = 0
$alreadyUtf8 = 0
$unicodeStripped = 0
$errors = 0

foreach ($file in $mdFiles) {
    $relativePath = $file.FullName.Replace((Get-Location).Path + '\', '')
    
    try {
        # Read file bytes to check encoding
        $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
        $hasUtf8Bom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
        
        # Read content
        if ($hasUtf8Bom) {
            # Already has UTF-8 BOM - read as UTF-8
            $content = [System.IO.File]::ReadAllText($file.FullName, [System.Text.Encoding]::UTF8)
        } else {
            # Try UTF-8 first, fallback to Default (Windows-1251)
            try {
                $content = [System.IO.File]::ReadAllText($file.FullName, [System.Text.Encoding]::UTF8)
            } catch {
                $content = [System.IO.File]::ReadAllText($file.FullName, [System.Text.Encoding]::Default)
            }
        }
        
        $modified = $false
        
        # Replace Unicode symbols if requested
        if ($StripUnicode) {
            $originalContent = $content
            foreach ($unicode in $unicodeReplacements.Keys) {
                if ($content.Contains($unicode)) {
                    $content = $content.Replace($unicode, $unicodeReplacements[$unicode])
                    $modified = $true
                }
            }
            
            if ($modified) {
                if ($Verbose) {
                    Write-Host "  Unicode stripped: $relativePath" -ForegroundColor Cyan
                }
                $unicodeStripped++
            }
        }
        
        # Always ensure UTF-8 with BOM
        if (-not $hasUtf8Bom -or $modified) {
            if (-not $DryRun) {
                # Write as UTF-8 with BOM
                $utf8BomEncoding = [System.Text.UTF8Encoding]::new($true)
                [System.IO.File]::WriteAllText($file.FullName, $content, $utf8BomEncoding)
                
                # Verify
                $verifyBytes = [System.IO.File]::ReadAllBytes($file.FullName)
                $verifyBom = $verifyBytes.Length -ge 3 -and $verifyBytes[0] -eq 0xEF -and $verifyBytes[1] -eq 0xBB -and $verifyBytes[2] -eq 0xBF
                
                if ($verifyBom) {
                    Write-Host "  ✓ Converted: $relativePath" -ForegroundColor Green
                } else {
                    Write-Host "  ✗ FAILED to write BOM: $relativePath" -ForegroundColor Red
                    $errors++
                }
            } else {
                Write-Host "  → Would convert: $relativePath" -ForegroundColor Yellow
            }
            $converted++
        } else {
            if ($Verbose) {
                Write-Host "  ✓ Already UTF-8 BOM: $relativePath" -ForegroundColor Gray
            }
            $alreadyUtf8++
        }
    } catch {
        Write-Host "  ✗ Error: $relativePath - $($_.Exception.Message)" -ForegroundColor Red
        $errors++
    }
}

Write-Host ""
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Total files: $($mdFiles.Count)"
Write-Host "  Already UTF-8 BOM: $alreadyUtf8" -ForegroundColor Gray
Write-Host "  Converted: $converted" -ForegroundColor Green
if ($StripUnicode) {
    Write-Host "  Unicode symbols replaced: $unicodeStripped" -ForegroundColor Cyan
}
Write-Host "  Errors: $errors" -ForegroundColor $(if ($errors -gt 0) { "Red" } else { "Gray" })

if ($DryRun) {
    Write-Host ""
    Write-Host "DRY RUN - No changes applied" -ForegroundColor Yellow
    Write-Host "Run without -DryRun to apply changes" -ForegroundColor Yellow
}

Write-Host ""

# Recommend Unicode stripping if VS still has issues
if (-not $StripUnicode -and $converted -gt 0) {
    Write-Host "WORKAROUND: If Visual Studio STILL shows encoding issues:" -ForegroundColor Yellow
    Write-Host "  Run: .\fix-encoding.ps1 -StripUnicode" -ForegroundColor Cyan
    Write-Host "  This replaces Unicode symbols (✅) with ASCII ([OK])" -ForegroundColor Gray
    Write-Host ""
}

# Also create .gitattributes to ensure consistent handling
$gitattributesPath = ".gitattributes"
$gitattributesContent = @"
# Git attributes for ApertureCore

# Markdown files - UTF-8 with BOM
*.md text eol=crlf encoding=utf-8

# C++ source files - UTF-8
*.cpp text eol=crlf encoding=utf-8
*.h text eol=crlf encoding=utf-8
*.hpp text eol=crlf encoding=utf-8
*.cxx text eol=crlf encoding=utf-8

# CMake files - UTF-8
CMakeLists.txt text eol=crlf encoding=utf-8
*.cmake text eol=crlf encoding=utf-8

# PowerShell scripts - UTF-8 with BOM
*.ps1 text eol=crlf encoding=utf-8

# JSON files - UTF-8
*.json text eol=crlf encoding=utf-8

# Visual Studio files - UTF-8 with BOM
*.sln text eol=crlf encoding=utf-8
*.vcxproj text eol=crlf encoding=utf-8
*.vcxproj.filters text eol=crlf encoding=utf-8

# Doxygen configuration - UTF-8
Doxyfile* text eol=crlf encoding=utf-8
"@

if (-not $DryRun) {
    if (-not (Test-Path $gitattributesPath)) {
        Set-Content -Path $gitattributesPath -Value $gitattributesContent -Encoding UTF8
        Write-Host "Created .gitattributes" -ForegroundColor Green
    } else {
        Write-Host ".gitattributes already exists (not modified)" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
