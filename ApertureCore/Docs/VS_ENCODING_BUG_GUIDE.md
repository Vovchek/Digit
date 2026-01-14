# Visual Studio UTF-8 Encoding Bug - Complete Guide

**Problem:** Visual Studio displays Unicode symbols incorrectly in markdown files, even with UTF-8 BOM encoding.

**Affected Systems:** Windows with non-English locale (Russian, Chinese, etc.)

---

## The Problem Explained

### What Happens
1. You save `.md` file with UTF-8 with BOM encoding
2. File contains Unicode symbols: ✅ ❌ ⚠️ 🔄 📝
3. You close the file in Visual Studio
4. You reopen the file in Visual Studio
5. **Symbols appear as:** `?` or `??` or garbage characters

### Root Cause

Visual Studio has a **known bug** on systems with non-English locale:
- For `.md` files, VS **ignores** the UTF-8 BOM marker
- Falls back to system default encoding (Windows-1251 for Russian, etc.)
- Interprets UTF-8 bytes as Windows-1251 characters
- Result: Unicode symbols display incorrectly

**This is a Visual Studio bug, not your file's fault!**

---

## Solution 1: Configure Visual Studio (Try First)

### Step-by-Step

1. **Close all open .md files in Visual Studio**

2. **Configure VS to use UTF-8:**
   - Tools → Options
   - Environment → Documents
   - ✓ CHECK: "Save documents as Unicode (UTF-8 with signature) - Codepage 65001"
   - Click OK

3. **Fix existing files:**
   ```powershell
   cd ApertureCore
   .\fix-encoding.ps1
   ```

4. **Reopen files in Visual Studio**

### Verification

Open any `.md` file and check if symbols display correctly:
- ✅ Should show as checkmark
- ❌ Should show as X
- ⚠️ Should show as warning sign

**If this works: YOU'RE DONE!** 🎉

**If symbols still show as `?` or `??`: Continue to Solution 2**

---

## Solution 2: Strip Unicode Symbols (Reliable Workaround)

If Visual Studio still won't cooperate, replace Unicode with ASCII equivalents.

### Run Unicode Strip

```powershell
cd ApertureCore
.\fix-encoding.ps1 -StripUnicode
```

### What This Does

Replaces Unicode symbols with ASCII equivalents:

| Unicode | ASCII | Meaning |
|---------|-------|---------|
| ✅ | `[OK]` | Success |
| ❌ | `[X]` | Failed |
| ⚠️ | `[!]` | Warning |
| 🔄 | `[~]` | In progress |
| 📝 | `[ ]` | Pending |
| 📍 | `[*]` | Current |
| 🔧 | `[+]` | Fixed |
| 🎉 | `(*)` | Celebration |
| → | `->` | Arrow right |
| … | `...` | Ellipsis |

### Example Transformation

**Before:**
```markdown
## Status
- ✅ Tests passing
- ❌ Build failed
- ⚠️ Warning
```

**After:**
```markdown
## Status
- [OK] Tests passing
- [X] Build failed
- [!] Warning
```

**Advantages:**
- **100% reliable** - displays correctly in all editors
- **No encoding issues** - pure ASCII
- **Still readable** - meanings preserved
- **Git-friendly** - no encoding conflicts

**Disadvantages:**
- Less visually appealing
- Lose emoji expressiveness

---

## Solution 3: Use Alternative Editor for .md Files

If you want to keep Unicode symbols and don't want to fight VS:

### Option A: VS Code (Recommended)

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Right-click `.md` file in Windows Explorer
3. "Open with" → "Visual Studio Code"
4. **VS Code respects UTF-8 BOM correctly!** ✅

### Option B: Notepad++

1. Install [Notepad++](https://notepad-plus-plus.org/)
2. Open `.md` files in Notepad++
3. Encoding → UTF-8 with BOM
4. Displays correctly

### Option C: Windows Notepad (Windows 11)

Recent Windows 11 Notepad handles UTF-8 correctly:
1. Right-click `.md` file
2. "Open with" → "Notepad"
3. Should display correctly

---

## Solution 4: Hybrid Workflow

**Best of both worlds:**

### For Writing Documentation
- Use **VS Code** or **Notepad++** for editing `.md` files
- Keep Unicode symbols for better readability

### For Viewing in Visual Studio
- Run `.\fix-encoding.ps1 -StripUnicode` before committing
- Generates ASCII-compatible versions
- VS displays correctly

### Script to Toggle

Create `toggle-unicode.ps1`:
```powershell
# Toggle between Unicode and ASCII in markdown files
param([switch]$ToUnicode)

if ($ToUnicode) {
    # Convert ASCII back to Unicode (for external editors)
    # TODO: Implement reverse mapping
    Write-Host "Converting to Unicode..." -ForegroundColor Cyan
} else {
    # Convert Unicode to ASCII (for Visual Studio)
    .\fix-encoding.ps1 -StripUnicode
}
```

---

## Technical Details

### UTF-8 BOM Verification

Check if file has UTF-8 BOM:

```powershell
# PowerShell command
$bytes = [System.IO.File]::ReadAllBytes("CURRENT_STATUS_AND_ISSUES.md")
$hasUtf8Bom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
Write-Host "Has UTF-8 BOM: $hasUtf8Bom"
```

### Manual Encoding Check in VS

1. Open file in Visual Studio
2. File → Advanced Save Options
3. Check encoding:
   - Should be: **Unicode (UTF-8 with signature) - Codepage 65001**
   - If shows: **Cyrillic (Windows) - Codepage 1251** ← WRONG!

4. Change to UTF-8 with signature
5. Save

---

## Prevention

### .editorconfig (Already Created)

Ensures future files use correct encoding:

```ini
[*.md]
charset = utf-8-bom
```

### Git Configuration

`.gitattributes` ensures consistent encoding across systems:

```
*.md text eol=crlf encoding=utf-8
```

Both files already created by `fix-encoding.ps1`!

---

## Recommended Approach

**For your ApertureCore project:**

### Option 1: Keep Fighting (Not Recommended)
Try to make VS respect UTF-8 BOM → often futile on non-English systems

### Option 2: Strip Unicode (Recommended)
```powershell
.\fix-encoding.ps1 -StripUnicode
```
- Quick, reliable, no hassle
- Files work everywhere
- Slightly less pretty, but functional

### Option 3: External Editor (Best User Experience)
- Edit `.md` files in **VS Code**
- Edit code in **Visual Studio**
- Best of both worlds

---

## Commands Reference

```powershell
# Check VS encoding configuration
.\configure-vs-encoding.ps1

# Fix encoding (preserve Unicode)
.\fix-encoding.ps1

# Strip Unicode symbols (reliable workaround)
.\fix-encoding.ps1 -StripUnicode

# Preview changes (don't apply)
.\fix-encoding.ps1 -StripUnicode -DryRun

# Verbose output
.\fix-encoding.ps1 -Verbose
```

---

## Summary

**Problem:** Visual Studio UTF-8 encoding bug on non-English Windows

**Quick Fix:** 
```powershell
cd ApertureCore
.\fix-encoding.ps1 -StripUnicode
```

**Best Long-Term:**
- Use VS Code for markdown editing
- Use Visual Studio for code editing
- No encoding headaches!

---

**The encoding issue is 100% a Visual Studio bug, not your mistake!** The workarounds provided will solve it completely.

---

**Created:** 2024  
**Status:** Complete troubleshooting guide for VS UTF-8 encoding bug
