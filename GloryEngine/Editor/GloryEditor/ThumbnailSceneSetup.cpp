#include "ThumbnailSceneSetup.h"
#include "ThumbnailRenderer.h"

#include <GScene.h>
#include <Components.h>

namespace Glory::Editor
{
	void SetupMaterialScene(Entity root, UUID materialID)
	{
		Entity sphere = root.GetScene()->CreateEmptyObject("MaterialSphere");
		root.GetScene()->SetParent(sphere.GetEntityID(), root.GetEntityID());
		MeshRenderer& mesh = sphere.AddComponent<MeshRenderer>();
		mesh.m_Mesh.SetUUID(MaterialSphereMesh->GetUUID());
		mesh.m_Material.SetUUID(materialID);
	}
}
