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
		glm::vec2 parentSize{ float(width), float(height) };

        if (pRegistry->IsValid(parent))
        {
            UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
            startTransform = parentTransform.m_Transform;
			parentSize = { parentTransform.m_Width, parentTransform.m_Height };
        }

		Constraints::ProcessConstraint(pComponent.m_X, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, parentSize);
		Constraints::ProcessConstraint(pComponent.m_Y, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, parentSize);
		Constraints::ProcessConstraint(pComponent.m_Width, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, parentSize);
		Constraints::ProcessConstraint(pComponent.m_Height, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, parentSize);

		/* Conversion top to bottom rather than bottom to top */
		const float actualY = parentSize.y - float(pComponent.m_Y);
		const float actualYPivot = 1.0f - pComponent.m_Pivot.y;

		const glm::vec2 size{ pComponent.m_Width, pComponent.m_Height };
        const glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), pComponent.m_Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, actualY, pComponent.m_Depth));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(size.x, size.y, 0.0f));
		const glm::mat4 pivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Pivot.x*size.x, actualYPivot*size.y, 0.0f));
        pComponent.m_Transform = startTransform*translation*rotation*glm::inverse(pivotOffset)*scale;

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
		AssetManager& assets = pEngine->GetAssetManager();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		TextRenderData textData;
		textData.m_FontID = pComponent.m_Font.AssetUUID();
		textData.m_ObjectID = entity;
		textData.m_Text = pComponent.m_Text;
		textData.m_TextDirty = pComponent.m_Dirty;
		textData.m_Scale = pComponent.m_Scale;
		textData.m_Alignment = pComponent.m_Alignment;
		textData.m_TextWrap = pComponent.m_WrapWidth;
		textData.m_Color = pComponent.m_Color;
		pComponent.m_Dirty = false;

		FontData* pFont = pComponent.m_Font.Get(&assets);
		if (!pFont) return;
		MeshData* pMeshData = pDocument->GetTextMesh(textData, pFont);
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);

		ObjectData object;
		object.Model = transform.m_Transform;
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

	/*void ApplyPosConstraint(float& outValue, glm::vec4& parent, const PosConstraint& constraint)
	{
		if (!constraint.m_Enable) return;
		float start = 0.0f;
		switch (constraint.m_Alignment)
		{
		case ConstraintAlignment::Left:
			start = parent.x;
			break;
		case ConstraintAlignment::Center:
			start = parent.z/2.0f;
			break;
		case ConstraintAlignment::Right:
			start = parent.z;
			break;
		case ConstraintAlignment::Bottom:
			start = parent.y;
			break;
		case ConstraintAlignment::Middle:
			start = parent.w/2.0f;
			break;
		case ConstraintAlignment::Top:
			start = parent.w;
			break;
		default:
			break;
		}

		float value = constraint.m_Value;
		switch (constraint.m_Mode)
		{
		case ConstraintMode::SizePercentage:
			value = parent.x * value / 100.0f;
			break;
		case ConstraintMode::HalfSizePercentage:
			value = parent.x / 2.0f * value / 100.0f;
			break;
		default:
			break;
		}
		outValue = start + value;
	}*/

	/*void UIConstraintSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIConstraint& pComponent)
	{
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();

		uint32_t width, height;
		pDocument->GetUITexture()->GetDimensions(width, height);
		glm::vec4 parentRect{ 0.0f, 0.0f, float(width), float(height) };

		UITransform& transform = pRegistry->GetComponent<UITransform>(entity);
		const Utils::ECS::EntityID parent = pRegistry->GetParent(entity);
		if (parent)
		{
			const UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
			parentRect = parentTransform.m_Rect.m_Value;
			parentRect = { 0.0f, 0.0f, parentTransform.m_Rect.m_Value.z - parentTransform.m_Rect.m_Value.x,
				parentTransform.m_Rect.m_Value.w - parentTransform.m_Rect.m_Value.y };
		}

		ApplyPosConstraint(transform.m_Rect.x, transform.m_Rect, pComponent.m_X);
		ApplyPosConstraint(transform.m_Rect.y, transform.m_Rect, pComponent.m_Y);
	}*/
}
