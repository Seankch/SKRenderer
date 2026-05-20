#ifndef ENGINE_H
#define ENGINE_H

/*****************************************************
	Includes
*****************************************************/
#include "DIContainer/DIContainer.h"

class Engine
{
public:
	void Init();
	void Update();
	void Exit();

private:
	DIContainer diContainer{};
};

#endif