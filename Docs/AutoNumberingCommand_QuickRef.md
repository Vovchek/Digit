# AutoNumberingCommand - Quick Reference

## Keyboard Shortcut

| Context | Action |
|---------|--------|
| **Navigate Mode** | Press **'N'** to auto-number selected fringes |

## Selection Behavior

### With Selection
- **Selected segments** → Used as **trusted reference points**
- Algorithm infers numbers for all others based on constraints

### Without Selection (Default)
- **First and last segments** → Automatically marked as trusted
- Perfect for regular fringe patterns

## Example Workflows

### Quick Auto-Number (No Selection)
```
1. Switch to Navigate mode
2. Press 'N'
3. Done! Numbers assigned automatically
```

### Custom Reference Points
```
1. Navigate mode
2. Click segment #2 → Select
3. Ctrl+Click segment #8 → Add to selection
4. Ctrl+Click segment #14 → Add to selection
5. Press 'N'
6. Segments 2,8,14 used as constraints
```

### Validate & Iterate
```
1. Press 'N' → Auto-number
2. Check result visually
3. Not satisfied? Press Ctrl+Z → Undo
4. Select different segments
5. Press 'N' again → Re-number with new constraints
```

## Parameters

All parameters have sensible defaults:

| Parameter | Default | Meaning |
|-----------|---------|---------|
| **step** | 1.0 | Fringe spacing |
| **confidenceThreshold** | 0.7 | Min validation score |

For custom parameters, use `AutoNumberingCommand` directly in code.

## Algorithm Capabilities

✅ **Supports:**
- Parallel curves (horizontal, vertical, inclined)
- Evenly-spaced fringes
- Multiple fringe clusters with gaps
- Trusted constraint enforcement
- Undo/redo

⚠️ **Limitations (v1.0):**
- Concentric circles (add Phase 3.3 in v1.1)
- Single reference point (use 2+ for best results)

## Keyboard Shortcuts

| Key | Effect |
|-----|--------|
| **N** (Navigate) | Auto-number |
| **Ctrl+Z** | Undo numbering |
| **Ctrl+Y** | Redo numbering |
| **Click** | Select segment |
| **Ctrl+Click** | Add/toggle selection |

## Status Messages

Check `Output` window for diagnostics:
```
AutoNumberingCommand::Execute: 50 fringes numbered, 48 trusted
```

High "trusted" count = high confidence in result  
Lower count = some segments uncertain (but still numbered)

---

**Quick Start:** Navigate mode → Press 'N' → Done!
