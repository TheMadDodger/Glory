#include "Module.h"
#include "GloryContext.h"

namespace Glory
{
    Module::Module() : m_pEngine(nullptr)
    {
    }

    Module::~Module()
    {
    }

    Engine* Module::GetEngine()
    {
        return m_pEngine;
    }

    bool Module::HasPriority()
    {
        return false;
    }

    const ModuleMetaData& Module::GetMetaData() const
    {
        return m_MetaData;
    }

    void Module::SetMetaData(const ModuleMetaData& metaData)
    {
        m_MetaData = metaData;
        std::filesystem::path path = m_MetaData.Path();
        m_ModulePath = path.parent_path();
        int a = 0;
    }

    const std::filesystem::path& Module::GetPath() const
    {
        return m_ModulePath;
    }

    bool Module::GetResourcePath(const std::string& resource, std::filesystem::path& path) const
    {
        path = m_ModulePath;
        path.append("Resources").append(resource);
        return std::filesystem::exists(path);
    }

    void Module::AddScriptingExtender(IScriptExtender* pScriptExtender)
    {
        m_pScriptingExtender.push_back(pScriptExtender);
    }

    const Version& Module::ModuleVersion() const
    {
        return CoreVersion;
    }

    ModuleSettings& Module::Settings()
    {
        return m_Settings;
    }

    void Module::LoadSettings(const std::filesystem::path& settingsFile)
    {
        if (!std::filesystem::exists(settingsFile))
            m_Settings = YAML::Node(YAML::NodeType::Map);
        else m_Settings = YAML::LoadFile(settingsFile.string());
        LoadSettings(m_Settings);
    }
}