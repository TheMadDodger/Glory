#include "MeshRendererEditor.h"
#include "EditorApplication.h"
#include "EditorAssetManager.h"

#include <Engine.h>
#include <Renderer.h>
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
		EditorApplication* pApp = EditorApplication::GetInstance();

		Transform& transform = m_pComponentObject->GetRegistry()->GetComponent<Transform>(m_pComponentObject->EntityID());
		MeshRenderer& meshRenderer = GetTargetComponent();
		const UUID meshID = meshRenderer.m_Mesh.AssetUUID();
		Resource* pMeshResource = pApp->GetAssetManager().FindResource(meshID);
		if (pMeshResource)
		{
			MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
			const BoundingBox& boundingBox = pMeshData->GetBoundingBox();
			pApp->GetEngine()->ActiveRenderer()
				->DrawLineBox(transform.MatTransform, boundingBox.m_Center, boundingBox.m_HalfExtends, { 1, 1, 0, 1 });
		}
		return EntityComponentEditor::OnGUI();
	}

	std::string MeshRendererEditor::Name()
	{
		return "Mesh Renderer";
	}
}