#include "Managers/JSONManager.h"
#include <fstream>

#pragma warning(push, 0)
#include <prettywriter.h>
#include <ostreamwrapper.h>
#include <istreamwrapper.h>
#pragma warning(pop)

JSONManager::JSONManager()
{
}

JSONManager::~JSONManager()
{
}

void JSONManager::Load()
{
}

void JSONManager::LateLoad()
{
}

void JSONManager::Init()
{
}

void JSONManager::Update()
{

}

void JSONManager::FixedUpdate()
{

}

void JSONManager::LateUpdate()
{
}

void JSONManager::Exit()
{
}

void JSONManager::Unload()
{
}

void JSONManager::SetCurrentFile(rapidjson::Document* _file)
{
	mCurrentFile = _file;
}

//CLoses the currently loaded file
void JSONManager::CloseFile()
{
	mCurrentFile = nullptr;
}

template<>
bool JSONManager::Get<bool>(std::string _name)
{
	if (!mCurrentValue.empty())//currently opened _object
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return (*mCurrentValue.top())[_name.c_str()].GetBool();
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))//at the top layer of _object
			return (*mCurrentFile)[_name.c_str()].GetBool();
	}
	return false;
}

template<>
int JSONManager::Get<int>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return (*mCurrentValue.top())[_name.c_str()].GetInt();
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))
			return (*mCurrentFile)[_name.c_str()].GetInt();
	}
	return -1;
}

template<>
float JSONManager::Get<float>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return (*mCurrentValue.top())[_name.c_str()].GetFloat();
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))
			return (*mCurrentFile)[_name.c_str()].GetFloat();
	}
	return -1.0f;
}

template<>
double JSONManager::Get<double>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return (*mCurrentValue.top())[_name.c_str()].GetDouble();
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))
			return (*mCurrentFile)[_name.c_str()].GetDouble();
	}
	return -1.0;
}

template<>
glm::vec2 JSONManager::Get<glm::vec2>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return glm::vec2((*mCurrentValue.top())[_name.c_str()][0].GetFloat(), (*mCurrentValue.top())[_name.c_str()][1].GetFloat());
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))
			return glm::vec2((*mCurrentFile)[_name.c_str()][0].GetFloat(), (*mCurrentFile)[_name.c_str()][1].GetFloat());
	}
	return glm::vec2();
}

template<>
glm::vec3 JSONManager::Get<glm::vec3>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return glm::vec3((*mCurrentValue.top())[_name.c_str()][0].GetFloat(), (*mCurrentValue.top())[_name.c_str()][1].GetFloat(), (*mCurrentValue.top())[_name.c_str()][2].GetFloat());
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))
			return glm::vec3((*mCurrentFile)[_name.c_str()][0].GetFloat(), (*mCurrentFile)[_name.c_str()][1].GetFloat(), (*mCurrentFile)[_name.c_str()][2].GetFloat());
	}
	return glm::vec3();
}

template<>
glm::vec4 JSONManager::Get<glm::vec4>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->HasMember(_name.c_str()))
			return glm::vec4((*mCurrentValue.top())[_name.c_str()][0].GetFloat(), (*mCurrentValue.top())[_name.c_str()][1].GetFloat(), (*mCurrentValue.top())[_name.c_str()][2].GetFloat(), (*mCurrentValue.top())[_name.c_str()][3].GetFloat());
	}
	else
	{
		if (mCurrentFile && mCurrentFile->HasMember(_name.c_str()))
			return glm::vec4((*mCurrentFile)[_name.c_str()][0].GetFloat(), (*mCurrentFile)[_name.c_str()][1].GetFloat(), (*mCurrentFile)[_name.c_str()][2].GetFloat(), (*mCurrentFile)[_name.c_str()][3].GetFloat());
	}
	return glm::vec4();
}

template<>
std::string JSONManager::Get<std::string>(std::string _name)
{
	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->IsObject() && mCurrentValue.top()->HasMember(_name.c_str()))
			return (*mCurrentValue.top())[_name.c_str()].GetString();
	}
	else
	{
		if (mCurrentFile && mCurrentFile->IsObject() && mCurrentFile->HasMember(_name.c_str()))
			return (*mCurrentFile)[_name.c_str()].GetString();
	}
	return "";
}

rapidjson::Value* JSONManager::GetValue(std::string _name)
{
	if (mCurrentFile->IsObject())
	{
		if (mCurrentFile && mCurrentFile->IsObject() && mCurrentFile->HasMember(_name.c_str()))
			return &(*mCurrentFile)[_name];
		else if (!mCurrentValue.empty())//currently opened _object
		{
			if (mCurrentValue.top()->HasMember(_name.c_str()))
				return &(*mCurrentValue.top())[_name];
		}
	}
	return nullptr;
}

void JSONManager::CreateNewSave()
{
	mCurrentFile = new rapidjson::Document;
	mCurrentFile->SetObject();
}

void JSONManager::DeleteNewSave()
{
	delete mCurrentFile;
	mCurrentFile = nullptr;
}

bool JSONManager::Save(std::string _name, glm::vec2 _val)
{
	rapidjson::Value _nameObj(_name.c_str(), mCurrentFile->GetAllocator());
	//double check for nan (set to 0?)
	if (std::isnan(_val.x))
		_val.x = 0;
	if (std::isnan(_val.y))
		_val.y = 0;
	//create array
	rapidjson::Value saveArr;
	saveArr.SetArray();
	saveArr.PushBack(_val.x, mCurrentFile->GetAllocator());
	saveArr.PushBack(_val.y, mCurrentFile->GetAllocator());
	if (!mCurrentValue.empty())//currently opened _object
	{
		mCurrentValue.top()->AddMember(_nameObj.Move(), saveArr.Move(), mCurrentFile->GetAllocator());
		return true;
	}
	else
	{
		if (mCurrentFile)//at the top layer of _object
		{
			mCurrentFile->AddMember(_nameObj.Move(), saveArr.Move(), mCurrentFile->GetAllocator());
			return true;
		}
	}

	return false;
}

bool JSONManager::Save(std::string _name, glm::vec3 _val)
{
	rapidjson::Value _nameObj(_name.c_str(), mCurrentFile->GetAllocator());
	//double check for nan (set to 0?)
	if (std::isnan(_val.x))
		_val.x = 0;
	if (std::isnan(_val.y))
		_val.y = 0;
	if (std::isnan(_val.z))
		_val.z = 0;
	//create array
	rapidjson::Value saveArr;
	saveArr.SetArray();
	saveArr.PushBack(_val.x, mCurrentFile->GetAllocator());
	saveArr.PushBack(_val.y, mCurrentFile->GetAllocator());
	saveArr.PushBack(_val.z, mCurrentFile->GetAllocator());
	if (!mCurrentValue.empty())//currently opened _object
	{
		mCurrentValue.top()->AddMember(_nameObj.Move(), saveArr.Move(), mCurrentFile->GetAllocator());
		return true;
	}
	else
	{
		if (mCurrentFile)//at the top layer of _object
		{
			mCurrentFile->AddMember(_nameObj.Move(), saveArr.Move(), mCurrentFile->GetAllocator());
			return true;
		}
	}

	return false;
}

bool JSONManager::Save(std::string _name, glm::vec4 _val)
{
	rapidjson::Value _nameObj(_name.c_str(), mCurrentFile->GetAllocator());
	//double check for nan (set to 0?)
	if (std::isnan(_val.x))
		_val.x = 0;
	if (std::isnan(_val.y))
		_val.y = 0;
	if (std::isnan(_val.z))
		_val.z = 0;
	if (std::isnan(_val.w))
		_val.w = 0;

	//create array
	rapidjson::Value saveArr;
	saveArr.SetArray();
	saveArr.PushBack(_val.x, mCurrentFile->GetAllocator());
	saveArr.PushBack(_val.y, mCurrentFile->GetAllocator());
	saveArr.PushBack(_val.z, mCurrentFile->GetAllocator());
	saveArr.PushBack(_val.w, mCurrentFile->GetAllocator());

	if (!mCurrentValue.empty())
	{
		mCurrentValue.top()->AddMember(_nameObj.Move(), saveArr.Move(), mCurrentFile->GetAllocator());
		return true;
	}
	else
	{
		if (mCurrentFile)
		{
			mCurrentFile->AddMember(_nameObj.Move(), saveArr.Move(), mCurrentFile->GetAllocator());
			return true;
		}
	}

	return false;
}

bool JSONManager::Save(std::string _name, std::string _val)
{
	return Save(_name, _val.c_str());
}

bool JSONManager::Save(std::string _name, const char* _val)
{
	rapidjson::Value _nameObj(_name.c_str(), mCurrentFile->GetAllocator());
	rapidjson::Value _valObj(_val, static_cast<rapidjson::SizeType>(strlen(_val)), mCurrentFile->GetAllocator());

	if (!mCurrentValue.empty())
	{
		if (mCurrentValue.top()->IsObject())
			mCurrentValue.top()->AddMember(_nameObj.Move(), _valObj.Move(), mCurrentFile->GetAllocator());
		else
			mCurrentValue.top()->PushBack(_valObj.Move(), mCurrentFile->GetAllocator());
		return true;
	}
	else
	{
		if (mCurrentFile)
		{
			mCurrentFile->AddMember(_nameObj.Move(), _valObj.Move(), mCurrentFile->GetAllocator());
			return true;
		}
	}

	return false;
}

bool JSONManager::SaveFile(std::string _fileName,std::string _extension,std::string _saveLocation)
{
	if (!mCurrentFile)//no open file
		return false;

	std::string fullPath = _saveLocation + _fileName + "." + _extension;
	std::ofstream outputFile;
	outputFile.open(fullPath);
	if (!outputFile.good())
	{
		return false;
	}
	rapidjson::OStreamWrapper osw(outputFile);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	mCurrentFile->Accept(writer);
	outputFile.close();

	return true;
}
