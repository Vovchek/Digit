# MFringe - Следующие шаги после перезапуска Visual Studio

## СРОЧНО: Исправить ошибку сборки MGTools (C1041)

### Вариант 1: Перезапуск + Clean (РЕКОМЕНДУЕТСЯ)
1. Закрыть Visual Studio
2. Открыть Visual Studio
3. Build → Clean Solution
4. Build → Rebuild Solution

### Вариант 2: Добавить флаг /FS в MGTools
Если Вариант 1 не помог, добавить в `MGTools/MGTools.vcxproj`:

Найдите секцию `<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">` 
и внутри блока `<ClCompile>` добавьте:

```xml
<AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>
```

То же самое для Release конфигурации.

---

## После успешной сборки MGTools:

### Step 2: Создать Core Domain Model

#### 2.1 Создать `MFringe/src/core/Units.h`
```cpp
enum class LengthUnit {
    Nanometers,  // nm
    Micrometers, // μm  
    Millimeters  // mm
    // NO meters!
};

class Units {
public:
    static double Convert(double value, LengthUnit from, LengthUnit to);
    static const char* GetSymbol(LengthUnit unit);
};
```

#### 2.2 Создать `MFringe/src/core/CoordinateSystem.h`
```cpp
enum class CoordinateSystemType {
    Image,      // Pixel coordinates
    Physical,   // Physical coordinates (mm/μm)
    Normalized  // [-1, 1] normalized
};
```

#### 2.3 Создать `MFringe/src/core/Measurement.h`
```cpp
class Measurement {
private:
    // Raw data
    std::shared_ptr<RawImage> m_rawImage;
    
    // Geometry
    std::shared_ptr<Aperture> m_aperture;
    
    // Processed data
    std::shared_ptr<PhaseData> m_phaseData;
    std::shared_ptr<WavefrontData> m_wavefront;
    
    // History for undo/redo
    std::vector<MeasurementSnapshot> m_history;
    
public:
    // One measurement = one physical measurement
    // No merging, no combining
};
```

#### 2.4 Создать Pipeline Architecture
- `MFringe/src/pipeline/PipelineStage.h`
- `MFringe/src/pipeline/PipelineManager.h`

---

## Проверить перед продолжением:

- [ ] Visual Studio перезапущена
- [ ] MGTools.lib успешно собирается
- [ ] InterfSolver.lib успешно собирается  
- [ ] MFringe компилируется без ошибок
- [ ] Все 3 .lib файла находятся в `Build\Debug\`

## Если всё работает, скажите в Copilot:

"продолжить с Step 2 - создать Measurement model"

или

"показать план создания Units и CoordinateSystem"

---

## Краткая справка по созданным файлам:

### Созданные MFC классы в MFringe:
- **CMFringeApp** (`src/mfc/MFringeApp.h/.cpp`) - главный класс приложения
- **CMFringeMainFrame** (`src/mfc/MainFrm.h/.cpp`) - главное окно (переименовано!)
- **CMFringeDoc** (`src/mfc/MFringeDoc.h/.cpp`) - документ
- **CMFringeView** (`src/mfc/MFringeView.h/.cpp`) - представление

### Исправленные конфликты:
- `CMainFrame` → `CMFringeMainFrame` (избежание конфликта с Digit)
- `CAboutDlg` → `CMFringeAboutDlg` (избежание конфликта с Digit)

### Структура проекта:
```
MFringe/
├── MFringe.vcxproj          ✅ Создан, пути исправлены
├── MFringe.vcxproj.filters  ✅ Создан
├── StdAfx.h/cpp             ✅ Precompiled headers
├── Resource.h               ✅ Определения ресурсов
├── MFringe.rc               ✅ Ресурсы
├── SETUP_HISTORY.md         📝 История настройки (ЭТОТ ФАЙЛ ВАЖЕН!)
├── VCXPROJ_FIX.md           📝 Инструкции по исправлению путей
├── TODO_AFTER_RESTART.md    📝 План действий (ВЫ ЗДЕСЬ)
└── src/
    └── mfc/
        ├── MFringeApp.h/.cpp      ✅ Класс приложения
        ├── MainFrm.h/.cpp         ✅ Главное окно
        ├── MFringeDoc.h/.cpp      ✅ Документ
        └── MFringeView.h/.cpp     ✅ Представление
```
