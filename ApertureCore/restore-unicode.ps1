# Restore Corrupted Unicode Symbols to ASCII
# Fixes ? and ?? artifacts in markdown files

param(
    [switch]$DryRun,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

Write-Host "Restore Corrupted Unicode to ASCII" -ForegroundColor Cyan
Write-Host "===================================" -ForegroundColor Cyan
Write-Host ""

# Comprehensive mapping of corrupted patterns to ASCII
# Based on common UTF-8 to Windows-1251 corruption patterns
$corruptionMappings = @{
    # Checkmarks and status symbols
    'â\x9C\x85' = '[OK]'     # ✅
    'âœ"' = '[x]'             # ✓
    'â\x9C\x93' = '[x]'      # ✓
    'â\x9D\x8C' = '[X]'      # ❌
    'â\x9A\xA0' = '[!]'      # ⚠
    'â\x9A\xA0ï¸\x8F' = '[!]' # ⚠️
    'ğŸ\x94\x84' = '[~]'     # 🔄
    'ğŸ"�' = '[ ]'            # 📝
    'ğŸ"�' = '[*]'            # 📍
    'ğŸ\x94§' = '[+]'        # 🔧
    'ğŸŽ‰' = '(*)'           # 🎉
    
    # Single character variants (common corruption)
    '?' = '[?]'              # Generic unknown
    '�' = '[?]'              # Replacement character
    
    # Common two-character corruptions
    '??' = '[ ]'             # Double question mark (often emoji)
    
    # Arrows
    'â†'' = '->'             # →
    'â\x86\x92' = '->'       # →
    'â\x86\x90' = '<-'       # ←
    'â\x86\x94' = '<->'      # ↔
    'â\x87\x92' = '=>'       # ⇒
    'â\x87\x90' = '<='       # ⇐
    'â\x87\x94' = '<=>'      # ⇔
    
    # Bullets and markers
    'â\x80¢' = '*'           # •
    'â\x97\x86' = '-'        # ◆
    'â\x96 ' = '#'           # ■
    'â\x96¡' = 'o'           # □
    'â\x96ª' = '-'           # ▪
    'â\x96«' = 'o'           # ▫
    
    # Special characters
    'â€¦' = '...'            # …
    'â€"' = '-'              # –
    'â€"' = '--'             # —
    'â€™' = "'"              # '
    'â€˜' = "'"              # '
    'â€\x9D' = '"'           # "
    'â€œ' = '"'              # "
    'Â«' = '<<'              # «
    'Â»' = '>>'              # »
    'Â©' = '(c)'             # ©
    'Â®' = '(R)'             # ®
    'â„¢' = '(TM)'           # ™
    
    # Tree/directory symbols
    'â"œ' = '+-'             # ├
    'â"€' = '-'              # ─
    'â""' = '+-'             # └
    'â"‚' = '|'              # │
    'â"�' = '+-'             # ┌
}

# Additional simple replacements for common patterns
$simpleReplacements = @{
    # Known corrupted status indicators from your files
    '? ' = '[OK] '
    '??' = '[ ]'
    '? ' = '[X] '
    '?? ' = '[!] '
    
    # Tree drawing characters commonly corrupted
    '├─' = '+-'
    '└─' = '+-'
    '│ ' = '| '
    '─ ' = '- '
}

# Get all markdown files
$mdFiles = Get-ChildItem -Path . -Filter "*.md" -Recurse | Where-Object { 
    $_.FullName -notmatch '\\out\\' -and 
    $_.FullName -notmatch '\\build\\' -and
    $_.FullName -notmatch '\\docs\\html\\' 
}

Write-Host "Found $($mdFiles.Count) markdown files to process" -ForegroundColor Green
Write-Host ""

$totalFixed = 0
$filesModified = 0

foreach ($file in $mdFiles) {
    $relativePath = $file.FullName.Replace((Get-Location).Path + '\', '')
    
    try {
        # Read as raw bytes first
        $content = [System.IO.File]::ReadAllText($file.FullName, [System.Text.Encoding]::UTF8)
        $originalContent = $content
        $fixCount = 0
        
        # Apply corruption mappings
        foreach ($corrupt in $corruptionMappings.Keys) {
            if ($content -match [regex]::Escape($corrupt)) {
                $count = ([regex]::Matches($content, [regex]::Escape($corrupt))).Count
                $content = $content -replace [regex]::Escape($corrupt), $corruptionMappings[$corrupt]
                $fixCount += $count
            }
        }
        
        # Apply simple replacements
        foreach ($pattern in $simpleReplacements.Keys) {
            if ($content.Contains($pattern)) {
                $beforeCount = $content.Length
                $content = $content.Replace($pattern, $simpleReplacements[$pattern])
                if ($content.Length -ne $beforeCount) {
                    $fixCount++
                }
            }
        }
        
        # Check if file was modified
        if ($content -ne $originalContent) {
            if (-not $DryRun) {
                # Write as UTF-8 with BOM
                $utf8BomEncoding = [System.Text.UTF8Encoding]::new($true)
                [System.IO.File]::WriteAllText($file.FullName, $content, $utf8BomEncoding)
                Write-Host "  [OK] Fixed $fixCount issues in: $relativePath" -ForegroundColor Green
            } else {
                Write-Host "  -> Would fix $fixCount issues in: $relativePath" -ForegroundColor Yellow
            }
            
            $totalFixed += $fixCount
            $filesModified++
            
            if ($Verbose) {
                Write-Host "       Changes: $fixCount replacements" -ForegroundColor Gray
            }
        } else {
            if ($Verbose) {
                Write-Host "  [x] No changes needed: $relativePath" -ForegroundColor Gray
            }
        }
    } catch {
        Write-Host "  [X] Error processing: $relativePath - $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Files scanned: $($mdFiles.Count)"
Write-Host "  Files modified: $filesModified" -ForegroundColor $(if ($filesModified -gt 0) { "Green" } else { "Gray" })
Write-Host "  Total fixes: $totalFixed" -ForegroundColor $(if ($totalFixed -gt 0) { "Green" } else { "Gray" })

if ($DryRun) {
    Write-Host ""
    Write-Host "DRY RUN - No changes applied" -ForegroundColor Yellow
    Write-Host "Run without -DryRun to apply changes" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green

# Show what symbols were replaced
if ($totalFixed -gt 0 -and -not $DryRun) {
    Write-Host ""
    Write-Host "Common replacements made:" -ForegroundColor Cyan
    Write-Host "  ? or ✅ -> [OK]" -ForegroundColor Gray
    Write-Host "  ? or ❌ -> [X]" -ForegroundColor Gray
    Write-Host "  ?? or ⚠️ -> [!]" -ForegroundColor Gray
    Write-Host "  ?? or 🔄 -> [~]" -ForegroundColor Gray
    Write-Host "  ?? or 📝 -> [ ]" -ForegroundColor Gray
    Write-Host "  ?? or 📍 -> [*]" -ForegroundColor Gray
}
