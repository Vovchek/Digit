# Изменения в MFringe.vcxproj для исправления путей к библиотекам

## Проблема:
При сборке MFringe линкер не находил MGTools.lib, потому что искал в неправильной папке.

## Причина:
`$(SolutionDir)` при сборке отдельного проекта разрешается как папка проекта, а не корень решения.

## Решение:
Заменить абсолютные пути на относительные.

### Файл: MFringe/MFringe.vcxproj

**Строка 86 (Release):**
```xml
<!-- БЫЛО: -->
<AdditionalLibraryDirectories>$(SolutionDir)Build\Release;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>

<!-- СТАЛО: -->
<AdditionalLibraryDirectories>..\Build\Release;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

**Строка 125 (Debug):**
```xml
<!-- БЫЛО: -->
<AdditionalLibraryDirectories>$(SolutionDir)Build\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>

<!-- СТАЛО: -->
<AdditionalLibraryDirectories>..\Build\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

## Как применить вручную (если нужно):

1. Откройте `MFringe\MFringe.vcxproj` в текстовом редакторе
2. Найдите строки с `<AdditionalLibraryDirectories>`
3. Замените `$(SolutionDir)Build\` на `..\Build\`
4. Сохраните файл
5. Перезагрузите проект в Visual Studio

## PowerShell команда для автоматического применения:
```powershell
(Get-Content "MFringe\MFringe.vcxproj") -replace '\$\(SolutionDir\)Build\\', '..\Build\' | Set-Content "MFringe\MFringe.vcxproj"
```

## Проверка что изменения применены:
```powershell
Select-String -Path "MFringe\MFringe.vcxproj" -Pattern "AdditionalLibraryDirectories"
```

Должно показать:
```
MFringe\MFringe.vcxproj:86:      <AdditionalLibraryDirectories>..\Build\Release;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
MFringe\MFringe.vcxproj:125:      <AdditionalLibraryDirectories>..\Build\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```
