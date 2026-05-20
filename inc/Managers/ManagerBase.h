#ifndef MANAGERBASE_H
#define MANAGERBASE_H

class ManagerBase
{
public:
	// Base manager Functions
	virtual ~ManagerBase() {}
	virtual void Load() = 0;
	virtual void LateLoad() = 0;
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void FixedUpdate() = 0;
	virtual void LateUpdate() = 0;
	virtual void Exit() = 0;
	virtual void Unload() = 0;
};

#endif