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
    }

    std::string Glory::MeshFilterSystem::Name()
    {
        return "Mesh Filter";
    }
}
