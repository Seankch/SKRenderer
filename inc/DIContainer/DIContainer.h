#ifndef DICONTAINER_H
#define DICONTAINER_H

/*****************************************************
    Includes
*****************************************************/
#include <typeindex>
#include <map>
#include <vector>
#include <type_traits>
#include <Managers/ManagerBase.h>

class DIContainer
{
public:
    // Base functions
    DIContainer();
    ~DIContainer();
    void Clear();
    std::vector<ManagerBase*>& GetContainerList(void);

    // Template functions to add to/get from container
    template<typename T>
    T* Get();
    template<typename T>
    void Add(T* instance);
private:
    // Container storing all classes
    std::map<std::type_index, ManagerBase*> container;
    std::vector<ManagerBase*> containerList;
};

template<typename T>
inline typename T* DIContainer::Get()
{
    auto it = container.find(typeid(T));
    if (it == container.end())
        return nullptr;

    return static_cast<T*>(it->second);
}

template<typename T>
inline void DIContainer::Add(T* _instance)
{
    container.emplace(typeid(T), _instance);
    containerList.emplace_back(_instance);
}

#endif
