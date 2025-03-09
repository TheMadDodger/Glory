#include "UISystems.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIRendererModule.h"
#include "Constraints.h"

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
        const Utils::ECS::EntityID parent = pEntityView->Parent();

        if (pRegistry->IsValid(parent) && pRegistry->IsEntityDirty(parent))
        {
            CalculateMatrix(pRegistry, parent, pRegistry->GetComponent<UITransform>(parent));
        }

		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		uint32_t width, height;
		pDocument->GetUITexture()->GetDimensions(width, height);
		pComponent.m_ParentSize = { float(width), float(height) };

        if (pRegistry->IsValid(parent))
        {
            UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
			startTransform = parentTransform.m_TransformNoScaleNoPivot;
			pComponent.m_ParentSize = { parentTransform.m_Width, parentTransform.m_Height };
        }

		Constraints::ProcessConstraint(pComponent.m_X, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize);
		Constraints::ProcessConstraint(pComponent.m_Y, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize);
		Constraints::ProcessConstraint(pComponent.m_Width, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize);
		Constraints::ProcessConstraint(pComponent.m_Height, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize);

		/* Conversion top to bottom rather than bottom to top */
		const float actualY = parent ? -float(pComponent.m_Y) : pComponent.m_ParentSize.y - float(pComponent.m_Y);
		const float actualYPivot = 1.0f - pComponent.m_Pivot.y;

		const glm::vec2 size{ pComponent.m_Width, pComponent.m_Height };
		const glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), -glm::radians(pComponent.m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, actualY, pComponent.m_Depth));
		const glm::mat4 hierarchyTranslation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, pComponent.m_Y, pComponent.m_Depth));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(size.x, size.y, 0.0f));
		const glm::mat4 selfScale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Scale.x, pComponent.m_Scale.y, 0.0f));
		const glm::mat4 pivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Pivot.x*size.x, actualYPivot*size.y, 0.0f));
        pComponent.m_Transform = startTransform*translation*rotation*selfScale*glm::inverse(pivotOffset)*scale;
        pComponent.m_TransformNoScale = startTransform*translation*rotation*selfScale*glm::inverse(pivotOffset);
        pComponent.m_TransformNoScaleNoPivot = startTransform*translation*rotation*selfScale;

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

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		MeshData* pMeshData = pUIRenderer->GetImageMesh();
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);
		ObjectData object;
		object.Model = transform.m_Transform;
		object.Projection = pDocument->Projection();

		Material* pMaterial = pGraphics->UseMaterial(pUIRenderer->PrepassMaterial());

		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("texSampler", pTexture);

		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
	}

    void UITextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent)
    {
        UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
        UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		AssetManager& assets = pEngine->GetAssetManager();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);
		const glm::vec2 size{ transform.m_Width, transform.m_Height };

		Constraints::ProcessConstraint(pComponent.m_Scale, size, transform.m_ParentSize);

		TextRenderData textData;
		textData.m_FontID = pComponent.m_Font.AssetUUID();
		textData.m_ObjectID = entity;
		textData.m_Text = pComponent.m_Text;
		textData.m_TextDirty = pComponent.m_Dirty;
		textData.m_Scale = float(pComponent.m_Scale);
		textData.m_Alignment = pComponent.m_Alignment;
		textData.m_TextWrap = float(transform.m_Width)*float(pComponent.m_Scale);
		textData.m_Color = pComponent.m_Color;
		pComponent.m_Dirty = false;

		FontData* pFont = pComponent.m_Font.Get(&assets);
		if (!pFont) return;
		MeshData* pMeshData = pDocument->GetTextMesh(textData, pFont);
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);

		const uint32_t height = pFont->FontHeight();
		glm::vec2 textOffset{ 0.0f, height*textData.m_Scale - size.y };
		switch (textData.m_Alignment)
		{
		case Alignment::Left:
			break;
		case Alignment::Center:
			textOffset.x = -size.x/2.0f;
			break;
		case Alignment::Right:
			textOffset.x = -size.x;
			break;
		default:
			break;
		}

		ObjectData object;
		const glm::mat4 matTextOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(textOffset, 0.0f));
		object.Model = transform.m_TransformNoScale*glm::inverse(matTextOffset);
		object.Projection = pDocument->Projection();

		Material* pMaterial = pGraphics->UseMaterial(pUIRenderer->TextPrepassMaterial());

		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		InternalTexture* pTextureData = pFont->GetGlyphTexture();
		if (!pTextureData) return;

		Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("textSampler", pTexture);

		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
    }

	void UITextSystem::OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent)
	{
		pComponent.m_Dirty = true;
	}
}
