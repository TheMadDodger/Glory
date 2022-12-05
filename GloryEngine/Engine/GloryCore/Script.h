#pragma once
#include "Resource.h"
#include "FileData.h"
#include "ScriptProperty.h"
#include <yaml-cpp/yaml.h>

namespace Glory
{
    class Script : public FileData
    {
    public:
        Script();
        Script(FileData* pFileData);
        virtual ~Script();

        virtual void Invoke(Object* pObject, const std::string& method, void** args) = 0;

        virtual bool IsBehaviour() = 0;

        virtual void SetValue(Object* pObject, const std::string& name, void* value) = 0;
        virtual void GetValue(Object* pObject, const std::string& name, void* value) = 0;

        virtual void LoadScriptProperties(std::vector<ScriptProperty>& scriptProperties, YAML::Node& data) = 0;
        virtual void SetPropertyValues(Object* pObject, YAML::Node& node) = 0;
        virtual void GetPropertyValues(Object* pObject, YAML::Node& node) = 0;
    };
}
