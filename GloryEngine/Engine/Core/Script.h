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

        virtual void Invoke(UUID objectID, UUID sceneID, const std::string& method, void** args) = 0;
        virtual void InvokeSafe(UUID objectID, UUID sceneID, const std::string& method, std::vector<void*>& args) = 0;

        virtual bool IsBehaviour() = 0;

        virtual void SetValue(UUID objectID, UUID sceneID, const std::string& name, void* value) = 0;
        virtual void GetValue(UUID objectID, UUID sceneID, const std::string& name, void* value) = 0;

        virtual void LoadScriptProperties(std::vector<ScriptProperty>& scriptProperties, YAML::Node& data) = 0;
        virtual void SetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node) = 0;
        virtual void GetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node) = 0;
    };
}
