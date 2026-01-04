# Quick Unicode Symbol Replacement
# Replaces corrupted symbols with ASCII in all markdown files

$files = Get-ChildItem -Path "ApertureCore" -Filter "*.md" -Recurse | Where-Object {
    $_.FullName -notmatch '\\out\\' -and 
    $_.FullName -notmatch '\\build\\' -and
    $_.FullName -notmatch '\\docs\\html\\'
}

$replacements = @{
    '?' = '[OK]'
    '?' = '[X]'
    '??' = '[!]'
    '??' = '[~]'
    '??' = '[ ]'
    '??' = '[*]'
    '??' = '[+]'
    '??' = '(*)'
    '???' = '+-'
    '???' = '+-'
    '???' = '|'
    '???' = '-'
    '???' = '+-'
}

foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw -Encoding UTF8
    $modified = $false
    
    foreach ($old in $replacements.Keys) {
        if ($content.Contains($old)) {
            $content = $content.Replace($old, $replacements[$old])
            $modified = $true
        }
    }
    
    if ($modified) {
        [System.IO.File]::WriteAllText($file.FullName, $content, [System.Text.UTF8Encoding]::new($true))
        Write-Host "[OK] Fixed: $($file.Name)" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
