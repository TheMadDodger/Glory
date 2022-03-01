#include "MeshFilterSystem.h"
#include <SerializedPropertyManager.h>

namespace Glory
{
    MeshFilterSystem::MeshFilterSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry)
    {
    }
    MeshFilterSystem::~MeshFilterSystem()
    {
    }

    void MeshFilterSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, MeshFilter& pComponent)
    {
        properties.push_back(SerializedPropertyManager::GetProperty<AssetReferencePropertyTemplate<ModelData>>(uuid, std::string("Model"), 0, &pComponent.m_pModelData));
    }

    std::string Glory::MeshFilterSystem::Name()
    {
        return "Mesh Filter";
    }
}
