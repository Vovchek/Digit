#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <iostream>
#include <map>

template<typename T>
class ObjectWithBiggerRef {
public:
    T originalValue;  // Original value (like 1,3,5, etc.)
    int trueValue;    // The actual numeric value we'll assign
    ObjectWithBiggerRef<T>* bigger;  // single reference to bigger object
    
    ObjectWithBiggerRef(T val) : originalValue(val), trueValue(0), bigger(nullptr) {}
};

template<typename T>
void assignTrueValuesWithCustomRoots(
    std::vector<ObjectWithBiggerRef<T>*>& objects,
    const std::unordered_map<ObjectWithBiggerRef<T>*, int>& rootValues) {
    
    // Build children mapping
    std::unordered_map<ObjectWithBiggerRef<T>*, std::vector<ObjectWithBiggerRef<T>*>> children;
    std::unordered_set<ObjectWithBiggerRef<T>*> nonRoots;
    
    for (auto* obj : objects) {
        if (obj->bigger) {
            children[obj->bigger].push_back(obj);
            nonRoots.insert(obj);
        }
    }
    
    // Find all roots
    std::vector<ObjectWithBiggerRef<T>*> roots;
    for (auto* obj : objects) {
        if (nonRoots.find(obj) == nonRoots.end()) {
            roots.push_back(obj);
        }
    }
    
    // Assign values to roots (using provided values or defaults)
    for (auto* root : roots) {
        auto it = rootValues.find(root);
        if (it != rootValues.end()) {
            root->trueValue = it->second;
        } else {
            // Default assignment if not specified
            static int defaultRootValue = 1000;
            root->trueValue = defaultRootValue;
            defaultRootValue += 100;
        }
    }
    
    // BFS to assign children values
    std::queue<ObjectWithBiggerRef<T>*> q;
    std::unordered_map<ObjectWithBiggerRef<T>*, bool> visited;
    
    for (auto* root : roots) {
        q.push(root);
        visited[root] = true;
    }
    
    while (!q.empty()) {
        auto* current = q.front();
        q.pop();
        
        for (auto* child : children[current]) {
            child->trueValue = current->trueValue - 1;
            q.push(child);
            visited[child] = true;
        }
    }
}
// Version that also sorts by true values
template<typename T>
std::vector<ObjectWithBiggerRef<T>*> sortAndAssignTrueValues(
    std::vector<ObjectWithBiggerRef<T>*>& objects,
    const std::unordered_map<ObjectWithBiggerRef<T>*, int>& rootValues) {
    
    // First assign true values
    assignTrueValuesWithCustomRoots(objects, rootValues);
    
    // Then sort by true values (ascending - smaller first)
    std::sort(objects.begin(), objects.end(),
        [](const auto* a, const auto* b) {
            return a->trueValue < b->trueValue;
        });
    
    return objects;
}

// Example usage with display
int main() {
    // Create objects
    auto* root1 = new ObjectWithBiggerRef<int>(5);      // Original value 5
    auto* child1a = new ObjectWithBiggerRef<int>(7);    // Original value 3
    auto* child1b = new ObjectWithBiggerRef<int>(3);    // Original value 3
    auto* grandchild1 = new ObjectWithBiggerRef<int>(1); // Original value 1
    
    auto* root2 = new ObjectWithBiggerRef<int>(8);      // Original value 8
    auto* child2 = new ObjectWithBiggerRef<int>(6);     // Original value 6
    auto* grandchild2 = new ObjectWithBiggerRef<int>(4); // Original value 4
    
    // Establish relationships
    child1a->bigger = root1;        // 7 -> 5
    child1b->bigger = root1;        // 3 -> 5
    grandchild1->bigger = child1a;   // 1 -> 7
    
    child2->bigger = root2;          // 6 -> 8
    grandchild2->bigger = child2;    // 4 -> 6
    
    std::vector<ObjectWithBiggerRef<int>*> objects = {
        root2, child1a, root1, grandchild1, child1b, child2, grandchild2
    };
    std::unordered_map<ObjectWithBiggerRef<int>*, int> rootValues = {
        {root1, 100},  // Assign root1 a true value of 100
        {root2, 200}   // Assign root2 a true value of 200
    };

    std::cout << "Before assignment:\n";
    for (auto* obj : objects) {
        std::cout << "Original: " << obj->originalValue;
        if (obj->bigger) {
            std::cout << " -> " << obj->bigger->originalValue;
        } else {
            std::cout << " (root)";
        }
        std::cout << "\n";
    }
    
    // Assign true values
    assignTrueValuesWithCustomRoots(objects, rootValues);
    
    std::cout << "\nAfter true value assignment:\n";
    for (auto* obj : objects) {
        std::cout << "Original: " << obj->originalValue 
                  << " → True value: " << obj->trueValue;
        if (obj->bigger) {
            std::cout << " (points to " << obj->bigger->originalValue 
                      << " which has true value " << obj->bigger->trueValue << ")";
        }
        std::cout << "\n";
    }
    
    // Sort by true values
    auto sorted = sortAndAssignTrueValues(objects, rootValues);
    
    std::cout << "\nSorted by true value (ascending):\n";
    for (auto* obj : sorted) {
        std::cout << "True: " << obj->trueValue 
                  << " (orig: " << obj->originalValue << ")";
        if (obj->bigger) {
            std::cout << " -> " << obj->bigger->trueValue;
        }
        std::cout << "\n";
    }
    
    // Verify the relationship A + 1 = B for all pointers
    std::cout << "\nVerifying A + 1 = B relationships:\n";
    bool allValid = true;
    for (auto* obj : objects) {
        if (obj->bigger) {
            if (obj->trueValue + 1 != obj->bigger->trueValue) {
                std::cout << "FAIL: " << obj->originalValue 
                          << " (" << obj->trueValue << ") + 1 != "
                          << obj->bigger->originalValue 
                          << " (" << obj->bigger->trueValue << ")\n";
                allValid = false;
            }
        }
    }
    if (allValid) {
        std::cout << "All relationships satisfy A + 1 = B\n";
    }
    
    // Clean up
    for (auto* obj : objects) {
        delete obj;
    }
    
    return 0;
}