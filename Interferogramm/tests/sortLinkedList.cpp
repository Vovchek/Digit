#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

template<typename T>
class ObjectWithBiggerRef {
public:
    T value;
    ObjectWithBiggerRef* bigger;  // points to the object that is "bigger by one"
    
    ObjectWithBiggerRef(T val) : value(val), bigger(nullptr) {}
};

template<typename T>
std::vector<ObjectWithBiggerRef<T>*> sortObjectsByChain(
    const std::vector<ObjectWithBiggerRef<T>*>& objects) {
    
    std::vector<ObjectWithBiggerRef<T>*> result;
    std::unordered_map<ObjectWithBiggerRef<T>*, bool> visited;
    
    // Find all potential starting points (objects that no one points to)
    std::unordered_set<ObjectWithBiggerRef<T>*> hasIncoming;
    for (auto* obj : objects) {
        if (obj->bigger) {
            hasIncoming.insert(obj->bigger);
        }
    }
    
    // Process each chain starting from its head
    for (auto* obj : objects) {
        // If this object has no incoming references, it's a chain head
        if (hasIncoming.find(obj) == hasIncoming.end() && !visited[obj]) {
            // Traverse the chain
            ObjectWithBiggerRef<T>* current = obj;
            while (current && !visited[current]) {
                result.push_back(current);
                visited[current] = true;
                current = current->bigger;
            }
        }
    }
    
    // Handle any remaining objects (in case of cycles or objects missed)
    for (auto* obj : objects) {
        if (!visited[obj]) {
            result.push_back(obj);
        }
    }
    
    return result;
}

// Example usage:
#include <iostream>

int main() {
    // Create objects
    auto* obj1 = new ObjectWithBiggerRef<int>(1);
    auto* obj2 = new ObjectWithBiggerRef<int>(2);
    auto* obj3 = new ObjectWithBiggerRef<int>(3);
    auto* obj5 = new ObjectWithBiggerRef<int>(5);
    auto* obj6 = new ObjectWithBiggerRef<int>(6);
    
    // Establish "bigger by one" relationships
    obj1->bigger = obj2;  // 1 -> 2
    obj2->bigger = obj3;  // 2 -> 3 (chain: 1,2,3)
    obj5->bigger = obj6;  // 5 -> 6 (chain: 5,6)
    
    std::vector<ObjectWithBiggerRef<int>*> objects = {obj2, obj5, obj1, obj3, obj6};
    
    auto sorted = sortObjectsByChain(objects);
    
    std::cout << "Sorted objects: ";
    for (auto* obj : sorted) {
        std::cout << obj->value << " ";
    }
    std::cout << std::endl;
    
    // Clean up
    for (auto* obj : objects) {
        delete obj;
    }
    
    return 0;
}