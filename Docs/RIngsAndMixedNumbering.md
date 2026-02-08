
## **Ключевые правила нумерации:**

1. **Кольца (замкнутые фринги):**
   - Внешнее кольцо: K = N
   - Внутреннее кольцо: K = N + 1
   - Еще глубже: K = N + 2 и т.д.

2. **Ленты (незамкнутые фринги) внутри кольца:**
   - Все ленты внутри одного кольца имеют **одинаковый** K
   - K ленты = K кольца + 1

3. **Ленты вне колец:**
   - Упорядочиваются по нормали
   - Нумеруются последовательно от базового K

## **Исправленная реализация:**

```cpp
class FringeNumberingSystem {
private:
    FringeHierarchy hierarchy;
    double fringeStep;
    
public:
    bool assignNumbers(std::vector<FringeNode>& nodes, double step);
    
private:
    // Новый рекурсивный метод с учетом лент внутри колец
    void assignNumbersRecursive(FringeTreeNode* node, double parentNumber);
    
    // Нумерация лент внутри конкретного кольца
    void numberBandsInsideRing(FringeTreeNode* ringNode, double ringNumber);
    
    // Нумерация лент без родительского кольца (корневые ленты)
    void numberRootBands(const std::vector<FringeTreeNode*>& rootBands, 
                        double baseNumber);
};

bool FringeNumberingSystem::assignNumbers(std::vector<FringeNode>& nodes, double step) {
    if (nodes.empty() || step <= 0) return false;
    fringeStep = step;
    
    // 1. Строим иерархию
    if (!hierarchy.buildHierarchy(nodes)) {
        return false;
    }
    
    const auto* root = hierarchy.getRoot();
    if (!root) return false;
    
    // 2. Определяем базовый номер (может быть из trusted anchor)
    double baseNumber = 0.0;
    for (const auto& node : nodes) {
        if (node.isTrusted) {
            baseNumber = node.knownValue;
            break;
        }
    }
    
    // 3. Разделяем корневые узлы на кольца и ленты
    std::vector<FringeTreeNode*> rootRings;
    std::vector<FringeTreeNode*> rootBands;
    
    for (const auto& child : root->children) {
        if (child->fringeData->isClosed) {
            rootRings.push_back(child.get());
        } else {
            rootBands.push_back(child.get());
        }
    }
    
    // 4. Нумеруем корневые кольца и их содержимое
    double currentRingNumber = baseNumber;
    for (auto* ring : rootRings) {
        assignNumbersRecursive(ring, currentRingNumber);
        // Если есть несколько корневых колец, они нумеруются независимо
        // currentRingNumber += fringeStep; // Раскомментировать если нужно
    }
    
    // 5. Нумеруем корневые ленты (не внутри колец)
    numberRootBands(rootBands, baseNumber);
    
    return true;
}

void FringeNumberingSystem::assignNumbersRecursive(FringeTreeNode* node, 
                                                  double parentNumber) {
    if (!node || !node->fringeData) return;
    
    // Вычисляем номер для текущего узла
    double currentNumber;
    if (node->parent->fringeData == nullptr) {
        // Это корневое кольцо (родитель - root)
        currentNumber = parentNumber;
    } else {
        // Это вложенное кольцо
        currentNumber = parentNumber + fringeStep;
    }
    
    // Присваиваем номер кольцу
    node->fringeData->assignedK = static_cast<int>(std::round(currentNumber / fringeStep));
    node->fringeData->isAssigned = true;
    
    // 1. Сначала нумеруем все ленты внутри этого кольца
    numberBandsInsideRing(node, currentNumber);
    
    // 2. Затем рекурсивно обрабатываем вложенные кольца
    for (auto& child : node->children) {
        if (child->fringeData->isClosed) {
            // Вложенное кольцо
            assignNumbersRecursive(child.get(), currentNumber);
        }
        // Ленты уже обработаны в numberBandsInsideRing
    }
}

void FringeNumberingSystem::numberBandsInsideRing(FringeTreeNode* ringNode, 
                                                 double ringNumber) {
    if (!ringNode || !ringNode->fringeData) return;
    
    // Находим все ленты-дети этого кольца
    std::vector<FringeNode*> bands;
    for (const auto& child : ringNode->children) {
        if (!child->fringeData->isClosed) {
            bands.push_back(child->fringeData);
        }
    }
    
    if (bands.empty()) return;
    
    // Номер для лент внутри кольца = номер кольца + 1 шаг
    double bandNumber = ringNumber + fringeStep;
    int bandK = static_cast<int>(std::round(bandNumber / fringeStep));
    
    // Присваиваем всем лентам одинаковый номер
    for (auto* band : bands) {
        band->assignedK = bandK;
        band->isAssigned = true;
        
        // Ленты могут иметь свои дочерние элементы?
        // Если да, то их нужно обработать отдельно
    }
    
    // Упорядочиваем ленты внутри кольца (для внутренней сортировки)
    orderBandsWithinRing(bands, ringNode->fringeData);
}

void FringeNumberingSystem::orderBandsWithinRing(std::vector<FringeNode*>& bands,
                                                const FringeNode* ring) {
    if (bands.size() < 2) return;
    
    // Упорядочиваем ленты по углу относительно центра кольца
    double centerX = ring->centroid_x;
    double centerY = ring->centroid_y;
    
    std::sort(bands.begin(), bands.end(),
        [centerX, centerY](const FringeNode* a, const FringeNode* b) {
            double angleA = std::atan2(a->centroid_y - centerY, 
                                      a->centroid_x - centerX);
            double angleB = std::atan2(b->centroid_y - centerY, 
                                      b->centroid_x - centerX);
            return angleA < angleB; // По часовой стрелке
        });
}

void FringeNumberingSystem::numberRootBands(
    const std::vector<FringeTreeNode*>& rootBands, 
    double baseNumber) {
    
    if (rootBands.empty()) return;
    
    // 1. Упорядочиваем корневые ленты по доминирующему направлению
    std::vector<FringeNode*> bandNodes;
    for (auto* node : rootBands) {
        bandNodes.push_back(node->fringeData);
    }
    
    // Вычисляем нормаль для упорядочивания
    std::pair<double, double> normal = computeDominantNormal(bandNodes);
    
    // 2. Сортируем по проекции на нормаль
    std::sort(bandNodes.begin(), bandNodes.end(),
        [&normal](const FringeNode* a, const FringeNode* b) {
            double projA = a->centroid_x * normal.first + 
                          a->centroid_y * normal.second;
            double projB = b->centroid_x * normal.first + 
                          b->centroid_y * normal.second;
            return projA < projB;
        });
    
    // 3. Нумеруем последовательно
    double current = baseNumber;
    for (auto* band : bandNodes) {
        band->assignedK = static_cast<int>(std::round(current / fringeStep));
        band->isAssigned = true;
        current += fringeStep;
    }
}
```

## **Вспомогательная функция для вычисления нормали:**

```cpp
std::pair<double, double> computeDominantNormal(
    const std::vector<FringeNode*>& bands) {
    
    if (bands.size() < 2) {
        return {1.0, 0.0}; // Направление по умолчанию
    }
    
    // Собираем центроиды для PCA
    std::vector<std::pair<double, double>> centroids;
    for (const auto* band : bands) {
        centroids.emplace_back(band->centroid_x, band->centroid_y);
    }
    
    // Ваша PCA-функция
    // return computeNormalDirection(centroids);
    
    // Временная заглушка
    return {1.0, 0.0};
}
```

## **Тестовые примеры:**

```cpp
void testNumbering() {
    // Пример 1: Кольцо с лентами внутри
    std::vector<FringeNode> nodes = {
        // Внешнее кольцо
        {.primaryIndex = 1, .isClosed = true, .centroid_x = 100, .centroid_y = 100, .assignedK = 0},
        
        // Ленты внутри кольца (все должны получить K+1)
        {.primaryIndex = 2, .isClosed = false, .centroid_x = 100, .centroid_y = 90, .assignedK = 0},
        {.primaryIndex = 3, .isClosed = false, .centroid_x = 110, .centroid_y = 100, .assignedK = 0},
        {.primaryIndex = 4, .isClosed = false, .centroid_x = 100, .centroid_y = 110, .assignedK = 0},
        
        // Внутреннее кольцо
        {.primaryIndex = 5, .isClosed = true, .centroid_x = 100, .centroid_y = 100, .assignedK = 0},
        
        // Ленты внутри внутреннего кольца
        {.primaryIndex = 6, .isClosed = false, .centroid_x = 100, .centroid_y = 95, .assignedK = 0},
    };
    
    // Установим bounding box для проверки вложенности
    // (в реальном коде они должны быть заданы)
    
    FringeNumberingSystem numbering;
    numbering.assignNumbers(nodes, 1.0); // Шаг = 1
    
    // Проверяем:
    // Кольцо 1 (ID=1): K = 0
    // Ленты 2,3,4: K = 1 (все одинаковые)
    // Кольцо 5 (ID=5): K = 1 (вложенное)
    // Лента 6 (ID=6): K = 2 (внутри вложенного кольца)
}
```

## **Визуализация структуры дерева:**

```cpp
void printNumberingTree(const FringeTreeNode* node, int indent = 0) {
    if (!node) return;
    
    std::string prefix(indent * 2, ' ');
    
    if (node->fringeData) {
        std::string type = node->fringeData->isClosed ? "RING" : "BAND";
        std::cout << prefix << type 
                  << " ID=" << node->fringeData->primaryIndex
                  << " K=" << node->fringeData->assignedK
                  << " (centroid: " << node->fringeData->centroid_x
                  << ", " << node->fringeData->centroid_y << ")\n";
    } else {
        std::cout << prefix << "[ROOT]\n";
    }
    
    // Сначала показываем кольца, потом ленты
    std::vector<const FringeTreeNode*> rings;
    std::vector<const FringeTreeNode*> bands;
    
    for (const auto& child : node->children) {
        if (child->fringeData->isClosed) {
            rings.push_back(child.get());
        } else {
            bands.push_back(child.get());
        }
    }
    
    for (const auto* ring : rings) {
        printNumberingTree(ring, indent + 1);
    }
    
    for (const auto* band : bands) {
        printNumberingTree(band, indent + 1);
    }
}
```

## **Ключевые особенности:**

1. **Ленты внутри кольца** получают **одинаковый** номер: K_кольца + 1
2. **Вложенные кольца** нумеруются рекурсивно: каждое следующее +1
3. **Ленты внутри вложенного кольца** также получают одинаковый номер
4. **Корневые ленты** (не внутри колец) нумеруются последовательно по нормали
5. **Структура дерева** сохраняет все связи для последующего анализа

Этот подход обеспечивает консистентную нумерацию, где все фринги внутри одного "уровня" кольца имеют одинаковый порядковый номер.