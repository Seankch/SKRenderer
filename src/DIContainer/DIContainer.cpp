#include "DIContainer/DIContainer.h"

DIContainer::DIContainer()
{
}

DIContainer::~DIContainer()
{
}

void DIContainer::Clear()
{
    containerList.clear();
    container.clear();
}

std::vector<ManagerBase*>& DIContainer::GetContainerList(void)
{
    return containerList;
}
