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
    auto* obj10 = new ObjectWithBiggerRef<int>(10);
    auto* obj20 = new ObjectWithBiggerRef<int>(20);
    auto* obj21 = new ObjectWithBiggerRef<int>(21);
    auto* obj30 = new ObjectWithBiggerRef<int>(30);
    auto* obj40 = new ObjectWithBiggerRef<int>(40);
    auto* obj41 = new ObjectWithBiggerRef<int>(41);
    auto* obj50 = new ObjectWithBiggerRef<int>(50);
    auto* obj60 = new ObjectWithBiggerRef<int>(60);
    
    // Establish "bigger by one" relationships
    obj10->bigger = obj20;  // 10 -> 20 (chain: 10,20,30,40,50,60)
    obj20->bigger = obj30;  // 20 -> 30
    obj30->bigger = obj40;  // 30 -> 40
    obj40->bigger = obj50;  // 40 -> 50
    obj50->bigger = obj60;  // 50 -> 60
    obj21->bigger = obj30;  // 21 -> 30 (chain: 21,30,40,50,60)
    obj41->bigger = obj50;  // 41 -> 50 (chain: 41,50,60)
    std::vector<ObjectWithBiggerRef<int>*> objects = 
        {obj10, obj20, obj21, obj41, obj50, obj10, obj30, obj60};
    
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