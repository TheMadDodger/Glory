#include "SoundMaterialImporter.h"

#include <YAML_GLM.h>

#include <SoundMaterialData.h>

namespace Glory::Editor
{
    std::string_view SoundMaterialImporter::Name() const
    {
        return "Sound Material Importer";
    }

    bool SoundMaterialImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        return extension.compare(".gsmat") == 0;
    }

    ImportedResource SoundMaterialImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
        Utils::YAMLFileRef file{ path };

        auto absorption = file["Absorption"];
        auto scattering = file["Scattering"];
        auto transmission = file["Transmission"];

        SoundMaterial material;
        if (absorption.Exists())
            material.m_Absorption = absorption.As<glm::vec3>();
        if (scattering.Exists())
            material.m_Scattering = scattering.As<float>();
        if (transmission.Exists())
            material.m_Transmission = transmission.As<glm::vec3>();

        SoundMaterialData* pMaterialData = new SoundMaterialData(std::move(material));
        return ImportedResource(path, pMaterialData);
    }

    bool SoundMaterialImporter::SaveResource(const std::filesystem::path& path, SoundMaterialData* pScript) const
    {
        return false;
    }
}
