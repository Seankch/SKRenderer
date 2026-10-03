#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stack>
#include "Managers/ManagerBase.h"

#pragma warning(push, 0)
#include <document.h>
#pragma warning(pop)

class JSONManager : public ManagerBase
{
public:
	// Constructors/Destructors
	JSONManager();
	~JSONManager();

	// Functions for graphics system
	void Load();
	void LateLoad();
	void Init();
	void Update();
	void FixedUpdate();
	void LateUpdate();
	void Exit();
	void Unload();

	// Set current file
	void SetCurrentFile(rapidjson::Document* _file);		
	void CloseFile();

	template<typename T>
	T Get(std::string _name) { return T{}; };
	template<>
	bool Get<bool>(std::string _name);
	template<>
	int Get<int>(std::string _name);
	template<>
	float Get<float>(std::string _name);
	template<>
	double Get<double>(std::string _name);
	template<>
	glm::vec2 Get<glm::vec2>(std::string _name);
	template<>
	glm::vec3 Get<glm::vec3>(std::string _name);
	template<>
	glm::vec4 Get<glm::vec4>(std::string _name);
	template<>
	std::string Get<std::string>(std::string _name);
	rapidjson::Value* GetValue(std::string _name);

	void CreateNewSave();
	void DeleteNewSave();
	template<typename T>
	bool Save(std::string _name,T _val);
	bool Save(std::string _name, glm::vec2 _val);
	bool Save(std::string _name, glm::vec3 _val);
	bool Save(std::string _name, glm::vec4 _val);
	bool Save(std::string _name, std::string _val);
	bool Save(std::string _name, const char* _val);
	bool SaveFile(std::string _fileName, std::string _extension, std::string _saveLocation);

private:
	std::stack<rapidjson::Value*> mCurrentValue;
	rapidjson::Document* mCurrentFile{ nullptr };
};

template<typename T>
inline bool JSONManager::Save(std::string _name, T _val)
{
	rapidjson::Value _nameObj(_name.c_str(), mCurrentFile->GetAllocator());
	if (!mCurrentValue.empty())//currently opened _object
	{
		mCurrentValue.top()->AddMember(_nameObj.Move(), _val, mCurrentFile->GetAllocator());
		return true;
	}
	else
	{
		if (mCurrentFile)//at the top layer of _object
		{
			mCurrentFile->AddMember(_nameObj.Move(), _val, mCurrentFile->GetAllocator());
			return true;
		}
	}
	return false;
}

#endif