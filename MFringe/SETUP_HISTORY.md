# MFringe Project Setup History

## Дата: 11 января 2026

### Выполненные действия:

#### 1. Создание структуры проекта MFringe
- Создан `MFringe/MFringe.vcxproj` - файл проекта
- Создан `MFringe/MFringe.vcxproj.filters` - фильтры
- Создан `MFringe/StdAfx.h` и `StdAfx.cpp` - precompiled headers
- Создан `MFringe/Resource.h` и `MFringe.rc` - ресурсы

#### 2. Созданы MFC классы:
- `MFringe/src/mfc/MFringeApp.h/.cpp` - класс приложения (CMFringeApp)
- `MFringe/src/mfc/MainFrm.h/.cpp` - главное окно (**CMFringeMainFrame** - переименовано чтобы избежать конфликта с Digit)
- `MFringe/src/mfc/MFringeDoc.h/.cpp` - документ (CMFringeDoc)
- `MFringe/src/mfc/MFringeView.h/.cpp` - представление (CMFringeView)

#### 3. Решённые проблемы:

##### Проблема 1: Конфликт имён классов
**Симптомы:** LNK2005 ошибки - дублирование CAboutDlg и CMainFrame между Digit и MFringe

**Решение:** 
- `CAboutDlg` → `CMFringeAboutDlg`
- `CMainFrame` → `CMFringeMainFrame`

##### Проблема 2: Ошибка RC1004 в Resource.h
**Симптомы:** "unexpected end of file found"

**Решение:** Добавлена пустая строка в конец файла Resource.h

##### Проблема 3: MGTools.lib не найдена при линковке MFringe
**Симптомы:** LNK1104: не удается открыть файл "MGTools.lib"

**Причина:** 
- `Digit.vcxproj` находится в корне решения, путь `Build\Debug` работает
- `MFringe.vcxproj` находится в подпапке `MFringe\`, путь `$(SolutionDir)Build\Debug` разрешается неправильно как `MFringe\Build\Debug`

**Исправление в MFringe.vcxproj:**
```xml
<!-- БЫЛО: -->
<AdditionalLibraryDirectories>$(SolutionDir)Build\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>

<!-- СТАЛО: -->
<AdditionalLibraryDirectories>..\Build\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

То же самое для Release:
```xml
<AdditionalLibraryDirectories>..\Build\Release;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

##### Проблема 4: C1041 ошибки в MGTools
**Симптомы:** "невозможно открыть базу данных программы MGTools.pdb; воспользуйтесь /FS"

**Причина:** Параллельная компиляция - несколько потоков пытаются писать в один .pdb файл одновременно

**Решение:** 
- Это временная проблема, обычно решается перезапуском Visual Studio
- Или добавить флаг `/FS` в настройки компилятора MGTools

### Текущее состояние проекта:

✅ MFringe.vcxproj создан и настроен
✅ Базовые MFC классы созданы
✅ Пути к библиотекам исправлены
⚠️ MGTools.lib не собирается из-за C1041 (требуется перезапуск VS)

### Следующие шаги после перезапуска VS:

1. **Очистить все проекты:**
   ```
   Build → Clean Solution
   ```

2. **Собрать в правильном порядке:**
   - MGTools (сначала)
   - InterfSolver
   - MFringe

3. **Если C1041 повторяется**, добавить `/FS` в MGTools.vcxproj:
   ```xml
   <ClCompile>
     <AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>
   </ClCompile>
   ```

### Архитектура MFringe (из спецификации):

- **Тип:** SDI (Single Document Interface)
- **Принцип:** One Measurement = One Physical Measurement
- **Единицы:** только nm, μm, mm (НЕ метры!)
- **Разделение ответственности:**
  - Views НЕ вычисляют
  - Pipeline nodes НЕ рисуют
  - Parameters редактируются в отдельных панелях

### Структура папок (запланировано):

```
MFringe/
├── src/
│   ├── core/          # Measurement, Units, CoordinateSystem
│   ├── pipeline/      # PipelineStage, PipelineManager
│   ├── geometry/      # Aperture (использует ApertureCore)
│   ├── mfc/          # MFC UI слой
│   └── io/           # File I/O
└── Doc/
    └── interferogram_processing_application_master_specification_for_copilot.md
```

### Важные заметки:

1. **ApertureCore** должен использоваться напрямую вместо InterfSolver XY... классов
2. Следовать новой логике visibility из ApertureCore
3. Использовать адаптеры для интеграции с существующими Digit/InterfSolver классами

---

## Для продолжения работы:

После перезапуска VS скажите "продолжить с Step 2" для создания:
- `Measurement.h/.cpp` - основной класс данных
- `Units.h` - система единиц
- `CoordinateSystem.h` - системы координат

Или скажите "исправить C1041 в MGTools" для решения проблемы компиляции.
