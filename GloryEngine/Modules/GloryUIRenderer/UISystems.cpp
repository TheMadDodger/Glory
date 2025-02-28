#include "UISystems.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIRendererModule.h"

#include <GScene.h>
#include <Engine.h>
#include <FontData.h>
#include <RendererModule.h>
#include <GraphicsModule.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/glm.hpp>

namespace Glory
{
	void UITransformSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent)
	{
        if (!pRegistry->IsEntityDirty(entity)) return;
        CalculateMatrix(pRegistry, entity, pComponent);
	}

    void UITransformSystem::CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent)
    {
        glm::mat4 startTransform = glm::identity<glm::mat4>();

        Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
        Utils::ECS::EntityID parent = pEntityView->Parent();

        if (pRegistry->IsValid(parent) && pRegistry->IsEntityDirty(parent))
        {
            CalculateMatrix(pRegistry, parent, pRegistry->GetComponent<UITransform>(parent));
        }

        if (pRegistry->IsValid(parent))
        {
            UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
            startTransform = parentTransform.m_Transform;
        }

        const glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), pComponent.m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Rect.x, pComponent.m_Rect.y, 0.0f));
        pComponent.m_Transform = startTransform * translation * rotation;

        pRegistry->SetEntityDirty(entity, false);
    }

	void UIImageSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIImage& pComponent)
	{
	}

	void UIImageSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIImage& pComponent)
	{
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		AssetManager& assets = pEngine->GetAssetManager();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		TextureData* pTextureData = pComponent.m_Image.Get(&assets);
		if (!pTextureData) return;

		ImageData* pImage = pTextureData->GetImageData(&assets);
		if (!pImage) return;

		const uint32_t width = pImage->GetWidth();
		const uint32_t height = pImage->GetHeight();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		MeshData* pMeshData = pUIRenderer->GetImageMesh();
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);

		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(float(width), float(height), 0.0f));

		ObjectData object;
		object.Model = transform.m_Transform*scale;
		object.Projection = pDocument->Projection();

		Material* pMaterial = pGraphics->UseMaterial(pUIRenderer->PrepassMaterial());

		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("textSampler", pTexture);

		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
	}

	void UITextSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent)
	{
		
	}

    void UITextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent)
    {
        UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
        UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		TextRenderData textData;
		textData.m_Alignment = Alignment::Left;
		textData.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		textData.m_Scale = 1.0f;
		textData.m_Text = pComponent.m_Text;
		textData.m_ObjectID = entity;
		textData.m_TextDirty = false;

		FontData* pFont = pUIRenderer->GetFont();
		if (!pFont) return;
		MeshData* pMeshData = pDocument->GetTextMesh(textData, pFont);
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);

		ObjectData object;
		object.Model = transform.m_Transform;
		object.Projection = pDocument->Projection();

		Material* pMaterial = pGraphics->UseMaterial(pUIRenderer->PrepassMaterial());

		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		InternalTexture* pTextureData = pFont->GetGlyphTexture();
		if (!pTextureData) return;

		Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("textSampler", pTexture);

		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
    }
}
