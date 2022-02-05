#include "MeshFilterSystem.h"

namespace Glory
{
    MeshFilterSystem::MeshFilterSystem(Registry* pRegistry) : EntitySystemTemplate(pRegistry)
    {
    }
    MeshFilterSystem::~MeshFilterSystem()
    {
    }

    void MeshFilterSystem::OnAcquireSerializedProperties(std::vector<SerializedProperty>& properties, MeshFilter& pComponent)
    {
        properties.push_back(AssetReferencePropertyTemplate<ModelData>("Model", &pComponent.m_pModelData));
    }

    std::string Glory::MeshFilterSystem::Name()
    {
        return "Mesh Filter";
    }
}
