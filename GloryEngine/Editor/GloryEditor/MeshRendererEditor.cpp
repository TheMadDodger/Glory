#include "MeshRendererEditor.h"
#include "EditorApplication.h"
#include "EditorAssetManager.h"

#include <Engine.h>
#include <RendererModule.h>
#include <Components.h>

namespace Glory::Editor
{
	MeshRendererEditor::MeshRendererEditor()
	{
	}

	MeshRendererEditor::~MeshRendererEditor()
	{
	}

	void MeshRendererEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool MeshRendererEditor::OnGUI()
	{
		Transform& transform = m_pComponentObject->GetRegistry()->GetComponent<Transform>(m_pComponentObject->EntityID());
		MeshRenderer& meshRenderer = GetTargetComponent();
		const UUID meshID = meshRenderer.m_Mesh.AssetUUID();
		Resource* pMeshResource = EditorApplication::GetInstance()->GetAssetManager().FindResource(meshID);
		if (!pMeshResource) return false;
		MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
		const BoundingBox& boundingBox = pMeshData->GetBoundingBox();
		EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>()->DrawLineBox(transform.MatTransform, boundingBox.m_Center, boundingBox.m_HalfExtends, {1, 1, 0, 1});
		return EntityComponentEditor::OnGUI();
	}

	std::string MeshRendererEditor::Name()
	{
		return "Mesh Renderer";
	}
}