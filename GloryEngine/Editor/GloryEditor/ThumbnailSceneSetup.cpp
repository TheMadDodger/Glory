#include "ThumbnailSceneSetup.h"
#include "ThumbnailsRenderer.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"
#include "EditorAssetDatabase.h"

#include <TransformManager.h>
#include <MeshRenderManager.h>

#include <GScene.h>
#include <Components.h>
#include <Resources.h>
#include <Renderer.h>
#include <GraphicsDevice.h>
#include <ImageData.h>
#include <InternalTexture.h>

namespace Glory::Editor
{
	void SetupMaterialScene(Entity root, UUID materialID)
	{
		Entity sphere = root.GetScene()->CreateEmptyObject("MaterialSphere");
		root.GetScene()->SetParent(sphere.GetEntityID(), root.GetEntityID());
		MeshRenderer& mesh = sphere.AddComponent<MeshRenderer>();
		mesh.m_Mesh.SetUUID(MaterialSphereMeshID);
		mesh.m_Material.SetUUID(materialID);
		sphere.GetComponent<Transform>().Rotation =
			glm::rotate(glm::identity<glm::quat>(), glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
	}

	bool CanRenderMaterial(UUID materialID)
	{
		EditorMaterialManager& materials = EditorApplication::GetInstance()->GetMaterialManager();
		Resources& resources = EditorApplication::GetInstance()->GetEngine()->GetResources();

		MaterialData* pMaterial = materials.GetMaterial(materialID);
		if (!pMaterial) return false;

		for (size_t i = 0; i < pMaterial->ResourceCount(); ++i)
		{
			const UUID resourceID = pMaterial->GetResourceUUIDPointer(i)->GetUUID();
			if (resourceID && EditorAssetDatabase::AssetExists(resourceID) &&
				!resources.GetResource(resourceID)) return false;
		}

		return true;
	}

	void SetupMeshScene(Entity root, UUID meshID)
	{
		Entity meshEntity = root.GetScene()->CreateEmptyObject("Mesh");
		root.GetScene()->SetParent(meshEntity.GetEntityID(), root.GetEntityID());
		MeshRenderer& mesh = meshEntity.AddComponent<MeshRenderer>();
		mesh.m_Mesh.SetUUID(meshID);
		mesh.m_Material.SetUUID(GreyMaterialID);

		const glm::quat rotation1 = glm::rotate(glm::identity<glm::quat>(), glm::radians(45.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
		const glm::quat rotation2 = glm::rotate(glm::identity<glm::quat>(), glm::radians(45.0f), glm::vec3{ -1.0f, 0.0f, 0.0f });
		root.GetComponent<Transform>().Rotation = rotation1*rotation2;
	}

	bool CanRenderMesh(UUID meshID)
	{
		if (!GreyMaterialID) return false;

		Resources& resources = EditorApplication::GetInstance()->GetEngine()->GetResources();
		return resources.GetResource(meshID);
	}

	void SetupImageScene(Entity root, UUID imageID)
	{
	}

	void CustomRenderImage(UUID imageID, GraphicsDevice* pDevice, Renderer* pRenderer, uint32_t frameIndex, CommandBufferHandle commandBuffer)
	{
		Resources& resources = EditorApplication::GetInstance()->GetEngine()->GetResources();
		Resource* pResource = resources.GetResource(imageID);
		ImageData* pImage = static_cast<ImageData*>(pResource);
		InternalTexture textureData = InternalTexture(pImage, false);
		TextureHandle texture = pDevice->CreateTexture(&textureData);

		DescriptorSetUpdateInfo writeInfo;
		writeInfo.m_Samplers.resize(1);
		writeInfo.m_Samplers[0].m_DescriptorIndex = 0;
		writeInfo.m_Samplers[0].m_TextureHandle = texture;
		pDevice->UpdateDescriptorSet(ImageSetHandles[frameIndex], writeInfo);

		RenderPassHandle renderPass = pRenderer->FinalColorRenderPass(frameIndex);
		pDevice->BeginRenderPass(commandBuffer, renderPass);
		pDevice->BeginPipeline(commandBuffer, ImagePipelineHandle);
		pDevice->BindDescriptorSets(commandBuffer, ImagePipelineHandle, { ImageSetHandles[frameIndex] });
		pDevice->SetViewport(commandBuffer, 0.0, 0.0, ThumbnailResolution.x, ThumbnailResolution.y);
		pDevice->SetScissor(commandBuffer, 0, 0, uint32_t(ThumbnailResolution.x), uint32_t(ThumbnailResolution.y));
		pDevice->DrawQuad(commandBuffer);
		pDevice->EndRenderPass(commandBuffer);
	}

	void CustomRenderTexture(UUID imageID, GraphicsDevice* pDevice, Renderer* pRenderer, uint32_t frameIndex, CommandBufferHandle commandBuffer)
	{
		Resources& resources = EditorApplication::GetInstance()->GetEngine()->GetResources();
		Resource* pResource = resources.GetResource(imageID);
		TextureData* pTexture = static_cast<TextureData*>(pResource);
		TextureHandle texture = pDevice->CreateTexture(pTexture);

		DescriptorSetUpdateInfo writeInfo;
		writeInfo.m_Samplers.resize(1);
		writeInfo.m_Samplers[0].m_DescriptorIndex = 0;
		writeInfo.m_Samplers[0].m_TextureHandle = texture;
		pDevice->UpdateDescriptorSet(ImageSetHandles[frameIndex], writeInfo);

		RenderPassHandle renderPass = pRenderer->FinalColorRenderPass(frameIndex);
		pDevice->BeginRenderPass(commandBuffer, renderPass);
		pDevice->BeginPipeline(commandBuffer, ImagePipelineHandle);
		pDevice->BindDescriptorSets(commandBuffer, ImagePipelineHandle, { ImageSetHandles[frameIndex] });
		pDevice->SetViewport(commandBuffer, 0.0, 0.0, ThumbnailResolution.x, ThumbnailResolution.y);
		pDevice->SetScissor(commandBuffer, 0, 0, uint32_t(ThumbnailResolution.x), uint32_t(ThumbnailResolution.y));
		pDevice->DrawQuad(commandBuffer);
		pDevice->EndRenderPass(commandBuffer);
	}
}
