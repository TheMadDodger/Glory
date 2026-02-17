#include "ThumbnailSceneSetup.h"
#include "ThumbnailRenderer.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"
#include "EditorAssetManager.h"

#include <GScene.h>
#include <Components.h>
#include <EditorAssetDatabase.h>

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

	bool CanRenderMaterial(UUID materialID)
	{
		EditorMaterialManager& materials = EditorApplication::GetInstance()->GetMaterialManager();
		EditorAssetManager& assets = EditorApplication::GetInstance()->GetAssetManager();

		MaterialData* pMaterial = materials.GetMaterial(materialID);
		if (!pMaterial) return false;

		for (size_t i = 0; i < pMaterial->ResourceCount(); ++i)
		{
			const UUID resourceID = pMaterial->GetResourceUUIDPointer(i)->AssetUUID();
			if (resourceID && EditorAssetDatabase::AssetExists(resourceID) &&
				!assets.FindResource(resourceID)) return false;
		}

		return true;
	}
}
