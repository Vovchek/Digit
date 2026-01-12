# ИНСТРУКЦИИ ДЛЯ COPILOT - ОБЯЗАТЕЛЬНО К ВЫПОЛНЕНИЮ

## ⛔ ЗАПРЕЩЕНО ТРОГАТЬ:

### Проекты (НЕ редактировать):
- `Digit.vcxproj`
- `MGTools.vcxproj`

### Папки и файлы (НЕ изменять без прямого указания):
- `Digit\ImageTempl\*.*`
- `Digit\ImageFeatures\*.*`
- `Digit\MainFrame\*.*`
- `Digit\Utils\*.*`
- `Digit\Tools\*.*`
- `Digit\Options\*.*`
- `Digit\MGTools\*.*`
- `Digit\InterfSolver\*.*`
- `Digit\DigitMode\*.*`
- `Digit\Controls\*.*`
- `Digit\*.cpp`
- `Digit\*.h`
- `Digit\*.rc`

## ✅ РАЗРЕШЕННАЯ ОБЛАСТЬ РАБОТЫ:

### Рабочая папка:
- **ТОЛЬКО `Digit\MFringe\`** и её подпапки

### Исключения (НЕ создавать в MFringe):
- Временные файлы построения (`.obj`, `.pch`, `.ilk`, `.pdb`)
- Выходные файлы (`.exe`, `.lib`, `.dll`)
- Эти файлы должны создаваться в `Build\Debug\` и `Build\Release\`

## 🎯 ТЕКУЩАЯ ЗАДАЧА:

Создать **полноценный скелет MFringe** как это делает **Visual Studio App Wizard**:

### Стиль:
- Visual Studio 2008 style
- С возможностью переключения стилей (Office 2007, VS 2010, etc.)

### Спецификации:
1. **UI Layout:** `MFringe\Doc\mfringe_application_screen_ui_layout_specification.md`
2. **Master Spec:** `MFringe\Doc\interferogram_processing_application_master_specification_for_copilot.md`

### Требования к скелету:
- Полноценная SDI (Single Document Interface) структура
- Меню, тулбары, доки, статус бар
- Базовая навигация и команды (File, Edit, View, Help)
- MFC Feature Pack features (docking panes, ribbons, themes)
- Готовность к добавлению domain logic

## ⚠️ ВАЖНЫЕ ОГРАНИЧЕНИЯ:

### При создании файлов:
1. **НЕ добавлять** ссылки на файлы из `MFringe` в проект `Digit.vcxproj`
2. **Все новые файлы** должны быть внутри `MFringe\`
3. **Избегать конфликтов** имён классов с Digit (префикс `CMFringe...`)

### При работе с проектами:
- Редактировать **ТОЛЬКО** `MFringe\MFringe.vcxproj`
- Не трогать зависимости других проектов
- Использовать относительные пути (`..\Build\Debug`)

## 📋 ТЕКУЩЕЕ СОСТОЯНИЕ MFringe:

### ✅ Уже создано:
- `MFringe.vcxproj` (базовая структура)
- `StdAfx.h/.cpp`
- `Resource.h`, `MFringe.rc`
- `src/mfc/MFringeApp.h/.cpp` (CMFringeApp)
- `src/mfc/MainFrm.h/.cpp` (CMFringeMainFrame)
- `src/mfc/MFringeDoc.h/.cpp` (CMFringeDoc)
- `src/mfc/MFringeView.h/.cpp` (CMFringeView)

### ⏳ Нужно добавить (App Wizard style):
- Тулбары и меню (полноценные)
- Docking panes (Property, Output, Tool палитры)
- Status bar с индикаторами
- Ribbon UI (опционально)
- Theme/Style switching
- Resource файлы (иконки, битмапы)
- Настройки и preferences
- Document templates

## 🔄 ПЕРЕД НАЧАЛОМ РАБОТЫ:

1. Прочитать спецификации из `MFringe\Doc\`
2. Понять структуру UI из layout specification
3. Создать план файлов для скелета
4. НЕ начинать без подтверждения пользователя

---

**Эти инструкции ОБЯЗАТЕЛЬНЫ для выполнения при любой работе с проектом!**
