#include "UISystems.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIRendererModule.h"
#include "Constraints.h"

#include <GScene.h>
#include <Engine.h>
#include <GameTime.h>
#include <FontData.h>
#include <RendererModule.h>
#include <GraphicsModule.h>
#include <SceneManager.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/glm.hpp>
#include <LocalizeModuleBase.h>

namespace Glory
{
	void UITransformSystem::OnPostUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent)
	{
        if (!pRegistry->IsEntityDirty(entity)) return;
        CalculateMatrix(pRegistry, entity, pComponent);
	}

    void UITransformSystem::CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent, bool calculateParentIfDirty)
    {
		glm::mat4 startTransform = glm::identity<glm::mat4>();
		glm::mat4 startInteractionTransform = glm::identity<glm::mat4>();

        Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
        const Utils::ECS::EntityID parent = pEntityView->Parent();

        if (pRegistry->IsValid(parent) && pRegistry->IsEntityDirty(parent) && calculateParentIfDirty)
        {
            CalculateMatrix(pRegistry, parent, pRegistry->GetComponent<UITransform>(parent));
        }

		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		uint32_t width, height;
		pDocument->GetUITexture()->GetDimensions(width, height);
		const glm::vec2 screenSize{ float(width), float(height) };

		pComponent.m_ParentSize = screenSize;

        if (pRegistry->IsValid(parent))
        {
            UITransform& parentTransform = pRegistry->GetComponent<UITransform>(parent);
			startTransform = parentTransform.m_TransformNoScaleNoPivot;
			startInteractionTransform = parentTransform.m_InteractionTransformNoPivot;
			pComponent.m_ParentSize = { parentTransform.m_Width, parentTransform.m_Height };
        }

		Constraints::ProcessConstraint(pComponent.m_Width, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
 		Constraints::ProcessConstraint(pComponent.m_Height, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		Constraints::ProcessConstraint(pComponent.m_Width, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		Constraints::ProcessConstraint(pComponent.m_X, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		Constraints::ProcessConstraint(pComponent.m_Y, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);
		Constraints::ProcessConstraint(pComponent.m_X, glm::vec2{ pComponent.m_Width, pComponent.m_Height }, pComponent.m_ParentSize, screenSize);

		/* Conversion top to bottom rather than bottom to top */
		const float actualY = parent ? -float(pComponent.m_Y) : pComponent.m_ParentSize.y - float(pComponent.m_Y);
		const float actualYPivot = 1.0f - pComponent.m_Pivot.y;

		const glm::vec2 size{ pComponent.m_Width, pComponent.m_Height };
		const glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), -glm::radians(pComponent.m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, actualY, 0.0f));
		const glm::mat4 interactionTranslation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_X, pComponent.m_Y, 0.0f));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(size.x, size.y, 1.0f));
		const glm::mat4 selfScale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Scale.x, pComponent.m_Scale.y, 1.0f));
		const glm::mat4 pivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Pivot.x*size.x, actualYPivot*size.y, 0.0f));
		const glm::mat4 interactionPivotOffset = glm::translate(glm::identity<glm::mat4>(), glm::vec3(pComponent.m_Pivot.x*size.x, pComponent.m_Pivot.y*size.y, 0.0f));
        pComponent.m_Transform = startTransform*translation*rotation*selfScale*glm::inverse(pivotOffset)*scale;
        pComponent.m_TransformNoScale = startTransform*translation*rotation*selfScale*glm::inverse(pivotOffset);
        pComponent.m_TransformNoScaleNoPivot = startTransform*translation*rotation*selfScale;
        pComponent.m_InteractionTransform = startInteractionTransform*interactionTranslation*rotation*glm::inverse(interactionPivotOffset)*selfScale;
        pComponent.m_InteractionTransformNoPivot = startInteractionTransform*interactionTranslation*rotation*selfScale;

        pRegistry->SetEntityDirty(entity, false);
		pDocument->SetDrawDirty();
    }

	void UIImageSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIImage& pComponent)
	{
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		AssetManager& assets = pEngine->GetAssetManager();
		MaterialManager& materials = pEngine->GetMaterialManager();
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

		MaterialData* pPrepassMaterial = pUIRenderer->PrepassMaterial();
		pPrepassMaterial->Set(materials, "Color", pComponent.m_Color);
		pPrepassMaterial->Set(materials, "HasTexture", glm::vec4{ 1.0f });

		Material* pMaterial = pGraphics->UseMaterial(pPrepassMaterial);
		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("texSampler", pTexture);

		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
	}

	void UIImageSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const UIImage* pImage = static_cast<const UIImage*>(pTypeView->GetComponentAddressFromIndex(i));
			const UUID image = pImage->m_Image.AssetUUID();
			if (image) references.push_back(image);
		}
	}

	void UITextSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent)
	{
		if (pComponent.m_LocalizeTerm.empty()) return;

		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		LocalizeModuleBase* pLocalize = pEngine->GetOptionalModule<LocalizeModuleBase>();
		if (!pLocalize) return;

		const std::string_view fullTerm = pComponent.m_LocalizeTerm;
		const size_t firstDot = fullTerm.find('.');
		if (firstDot == std::string::npos) return;
		const std::string_view tableName = fullTerm.substr(0, firstDot);
		const std::string_view term = fullTerm.substr(firstDot + 1);
		if (pLocalize->FindString(tableName, term, pComponent.m_Text))
		{
			pComponent.m_Dirty = true;
			pDocument->SetDrawDirty();
		}
	}

	void UITextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent)
    {
		/* No need to do anything if there is no text */
		if (pComponent.m_Text.empty()) return;
        UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
        UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		AssetManager& assets = pEngine->GetAssetManager();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);
		const glm::vec2 size{ transform.m_Width, transform.m_Height };

		uint32_t screenWidth, screenHeight;
		pDocument->GetUITexture()->GetDimensions(screenWidth, screenHeight);
		const glm::vec2 screenSize{ float(screenWidth), float(screenHeight) };

		Constraints::ProcessConstraint(pComponent.m_Scale, size, transform.m_ParentSize, screenSize);

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

	void UITextSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const UIText* pText = static_cast<const UIText*>(pTypeView->GetComponentAddressFromIndex(i));
			const UUID font = pText->m_Font.AssetUUID();
			if (font) references.push_back(font);
		}
	}

	void UIBoxSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIBox& pComponent)
	{
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		AssetManager& assets = pEngine->GetAssetManager();
		MaterialManager& materials = pEngine->GetMaterialManager();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		MeshData* pMeshData = pUIRenderer->GetImageMesh();
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);
		ObjectData object;
		object.Model = transform.m_Transform;
		object.Projection = pDocument->Projection();

		MaterialData* pPrepassMaterial = pUIRenderer->PrepassMaterial();
		pPrepassMaterial->Set(materials, "Color", pComponent.m_Color);
		pPrepassMaterial->Set(materials, "HasTexture", glm::vec4{ 0.0f });

		Material* pMaterial = pGraphics->UseMaterial(pPrepassMaterial);
		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
	}

	void UIInteractionSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIInteraction& pComponent)
	{
		if (!pComponent.m_Enabled) return;

		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		const bool inputAllowed = pDocument->IsEnputEnabled();
		if (!inputAllowed) return;

		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		const glm::vec4 cursor{ pDocument->GetCursorPos(), 0.0f, 1.0f };
		const glm::mat4 inverse = glm::inverse(transform.m_InteractionTransform);
		const glm::mat4 screenScaleTransform = glm::scale(glm::identity<glm::mat4>(), {});
		const glm::vec4 transformedCursor = inverse*cursor;

		const bool isMouseInRect = transformedCursor.x > 0.0f && transformedCursor.x < float(transform.m_Width) &&
			transformedCursor.y > 0.0f && transformedCursor.y < float(transform.m_Height);

		Utils::ECS::EntityView* pEntity = pRegistry->GetEntityView(entity);
		const UUID entityUUID = pDocument->EntityUUID(entity);
		const UUID componentID = pEntity->ComponentUUID(UIInteraction::GetTypeData()->TypeHash());
		const UUID sceneID = pDocument->SceneID();
		const UUID objectID = pDocument->ObjectID();

		const bool wasDown = pDocument->WasCursorDown();
		const bool isCursorDown = pDocument->IsCursorDown();

		if (isMouseInRect && !pComponent.m_Hovered)
		{
			pComponent.m_Hovered = true;
			if (!Instance()->OnElementHover_Callback) return;
			Instance()->OnElementHover_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}
		else if (!isMouseInRect && pComponent.m_Hovered)
		{
			pComponent.m_Hovered = false;
			if (!Instance()->OnElementUnHover_Callback) return;
			Instance()->OnElementUnHover_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}

		if (!wasDown && isCursorDown && pComponent.m_Hovered && !pComponent.m_Down)
		{
			pComponent.m_Down = true;
			if (!Instance()->OnElementDown_Callback) return;
			Instance()->OnElementDown_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}
		else if (!isCursorDown && pComponent.m_Down || pComponent.m_Down && !pComponent.m_Hovered)
		{
			pComponent.m_Down = false;
			if (!Instance()->OnElementUp_Callback) return;
			Instance()->OnElementUp_Callback(pEngine, sceneID, objectID, entityUUID, componentID);
		}
	}

	UIInteractionSystem* UIInteractionSystem::Instance()
	{
		static UIInteractionSystem Inst;
		return &Inst;
	}

	void UIPanelSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIPanel& pComponent)
	{
		if (!pComponent.m_Crop) return;
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		AssetManager& assets = pEngine->GetAssetManager();
		MaterialManager& materials = pEngine->GetMaterialManager();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		MeshData* pMeshData = pUIRenderer->GetImageMesh();
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);
		ObjectData object;
		object.Model = transform.m_Transform;
		object.Projection = pDocument->Projection();

		MaterialData* pPrepassMaterial = pUIRenderer->PrepassStencilMaterial();
		Material* pMaterial = pGraphics->UseMaterial(pPrepassMaterial);
		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		size_t& counter = pDocument->PanelCounter();
		pGraphics->SetColorMask(false, false, false, false);
		pGraphics->EnableStencilTest(true);
		pGraphics->SetStencilMask(0xFF);
		if (counter == 0)
			pGraphics->ClearStencil(0);

		/* First stencil pass increases stencil value by 1 */
		pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Increment);
		pGraphics->SetStencilFunc(CompareOp::OP_Always, counter, 0xFF);
		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());

		/* Second stencil pass compares stencil with the the expected addition, on fail it is reduced by 1 */
		pGraphics->SetStencilOP(Func::OP_Decrement, Func::OP_Decrement, Func::OP_Replace);
		pGraphics->SetStencilFunc(CompareOp::OP_Equal, counter + 1, 0xFF);
		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
		++counter;

		/* Disable writing to the stencil buffer, value must be equal to counter */
		pGraphics->SetColorMask(true, true, true, true);
		pGraphics->SetStencilMask(0x00);
		pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Keep);
		pGraphics->SetStencilFunc(CompareOp::OP_LessOrEqual, counter, 0xFF);
	}

	void UIPanelSystem::OnPostDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIPanel& pComponent)
	{
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		UIRendererModule* pUIRenderer = pDocument->Renderer();
		Engine* pEngine = pUIRenderer->GetEngine();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		size_t& counter = pDocument->PanelCounter();
		--counter;
		if (counter == 0)
		{
			pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Keep);
			pGraphics->SetStencilFunc(CompareOp::OP_Always, 0, 0xFF);
			pGraphics->SetStencilMask(0x00);
			pGraphics->EnableStencilTest(false);
			return;
		}

		if (!pComponent.m_Crop) return;
		const UITransform& transform = pRegistry->GetComponent<UITransform>(entity);
		MeshData* pMeshData = pUIRenderer->GetImageMesh();
		Mesh* pMesh = pResourceManager->CreateMesh(pMeshData);
		ObjectData object;
		object.Model = transform.m_Transform;
		object.Projection = pDocument->Projection();

		MaterialData* pPrepassMaterial = pUIRenderer->PrepassStencilMaterial();
		Material* pMaterial = pGraphics->UseMaterial(pPrepassMaterial);
		pMaterial->SetProperties(pEngine);
		pMaterial->SetObjectData(object);

		/* Remove the shape from the stencil */
		pGraphics->SetColorMask(false, false, false, false);
		pGraphics->SetStencilMask(0xFF);
		pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Decrement);
		pGraphics->SetStencilFunc(CompareOp::OP_Equal, counter + 1, 0xFF);
		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
		pGraphics->SetColorMask(true, true, true, true);

		/* Disable writing to the stencil buffer, value must be equal to counter */
		pGraphics->SetStencilMask(0x00);
		pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Keep);
		pGraphics->SetStencilFunc(CompareOp::OP_LessOrEqual, counter, 0xFF);
	}

	void UIVerticalContainerSystem::OnPreUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIVerticalContainer& pComponent)
	{
		if (!pComponent.m_Dirty) return;
		Utils::ECS::EntityView* pEntity = pRegistry->GetEntityView(entity);
		float height = 0.0f;
		for (size_t i = 0; i < pEntity->ChildCount(); ++i)
		{
			const Utils::ECS::EntityID child = pEntity->Child(i);
			UITransform& transform = pRegistry->GetComponent<UITransform>(child);
			if (pRegistry->IsEntityDirty(child))
				UITransformSystem::CalculateMatrix(pRegistry, child, transform, false);
			const float elementHeight = transform.m_Height.m_FinalValue;
			transform.m_Y.m_Constraint = 0;
			transform.m_Y.m_Value = height;
			height += elementHeight + pComponent.m_Seperation;
			UITransformSystem::CalculateMatrix(pRegistry, child, transform, false);
		}
		UITransform& transform = pRegistry->GetComponent<UITransform>(entity);
		if (pComponent.m_AutoResizeHeight && transform.m_Height.m_FinalValue != height)
		{
			transform.m_Height.m_Constraint = 0;
			transform.m_Height = height;
			UITransformSystem::CalculateMatrix(pRegistry, entity, transform, false);
		}
		pComponent.m_Dirty = false;
	}

	void UIVerticalContainerSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIVerticalContainer& pComponent)
	{
		pComponent.m_Dirty = false;
	}

	void UIVerticalContainerSystem::OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIVerticalContainer& pComponent)
	{
		pComponent.m_Dirty = true;
	}

	void UIScrollViewSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		pComponent.m_ScrollPosition = pComponent.m_StartScrollPosition;
		pComponent.m_Dirty = true;
	}

	void UIScrollViewSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		pComponent.m_ScrollPosition = pComponent.m_StartScrollPosition;
		pComponent.m_Dirty = true;
	}

	void UIScrollViewSystem::OnPreUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		if (!pRegistry->HasComponent<UIInteraction>(entity)) return;
		UIInteraction& interaction = pRegistry->GetComponent<UIInteraction>(entity);
		UIDocument* pDocument = pRegistry->GetUserData<UIDocument*>();
		Engine* pEngine = pDocument->Renderer()->GetEngine();

		const glm::vec2& cursorScrollDelta = pDocument->GetCursorScrollDelta();
		const float length = glm::length(cursorScrollDelta);
		if ((!pComponent.m_RequireHover || interaction.m_Hovered) && length != 0.0f)
		{
			if (!pComponent.m_LockHorizontal)
				pComponent.m_DesiredScrollPosition.x += pDocument->GetCursorScrollDelta().x*pComponent.m_ScrollSpeeds.x;
			if (!pComponent.m_LockVertical)
				pComponent.m_DesiredScrollPosition.y += pDocument->GetCursorScrollDelta().y*pComponent.m_ScrollSpeeds.y;
			pComponent.m_Dirty = true;
		}

		switch (pComponent.m_ScrollEdgeMode)
		{
		case ScrollEdgeMode::Clamp:
			pComponent.m_DesiredScrollPosition = glm::clamp(pComponent.m_DesiredScrollPosition, glm::vec2{ 0.0f, 0.0f }, pComponent.m_MaxScroll);
			break;
		case ScrollEdgeMode::Innertia:
		{
			const glm::vec2 target = glm::clamp(pComponent.m_DesiredScrollPosition, glm::vec2{ 0.0f, 0.0f }, pComponent.m_MaxScroll);
			pComponent.m_DesiredScrollPosition = glm::mix(pComponent.m_DesiredScrollPosition, target,
				pComponent.m_Innertia*pEngine->Time().GetDeltaTime());
			break;
		}
		default:
			break;
		}

		if (pComponent.m_ScrollPosition != pComponent.m_DesiredScrollPosition)
		{
			switch (pComponent.m_ScrollMode)
			{
			case ScrollMode::Snap:
				pComponent.m_ScrollPosition = pComponent.m_DesiredScrollPosition;
				pComponent.m_Dirty = true;
				break;
			case ScrollMode::Lerp:
				pComponent.m_ScrollPosition = glm::mix(pComponent.m_ScrollPosition, pComponent.m_DesiredScrollPosition,
					pComponent.m_LerpSpeed*pEngine->Time().GetDeltaTime());
				pComponent.m_Dirty = true;
				break;
			default:
				break;
			}
		}
	}

	void UIScrollViewSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		if (!pComponent.m_Dirty) return;

		/* Calculate content size */
		glm::vec2 contentSize{ 0.0f };
		glm::vec2 startPos{ 0.0f };
		for (size_t i = 0; i < pRegistry->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = pRegistry->Child(entity, i);
			UITransform& childTransform = pRegistry->GetComponent<UITransform>(child);
			if (pRegistry->IsEntityDirty(child))
				UITransformSystem::CalculateMatrix(pRegistry, child, childTransform, false);

			if (i == 0)
				startPos = glm::vec2{ childTransform.m_X.m_FinalValue, childTransform.m_Y.m_FinalValue };

			contentSize.x = glm::max(contentSize.x, childTransform.m_X.m_FinalValue - startPos.x + childTransform.m_Width.m_FinalValue);
			contentSize.y = glm::max(contentSize.y, childTransform.m_Y.m_FinalValue - startPos.y + childTransform.m_Height.m_FinalValue);
		}
		UITransform& transform = pRegistry->GetComponent<UITransform>(entity);

		const glm::vec2 tempMaxScroll = pComponent.m_MaxScroll;
		pComponent.m_MaxScroll = { glm::max(glm::vec2{0.0f}, contentSize - glm::vec2{ transform.m_Width.m_FinalValue, transform.m_Height.m_FinalValue }) };
		if (tempMaxScroll != pComponent.m_MaxScroll && pComponent.m_AutoScroll)
			pComponent.m_DesiredScrollPosition = pComponent.m_MaxScroll;

		for (size_t i = 0; i < pRegistry->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = pRegistry->Child(entity, i);
			UITransform& transform = pRegistry->GetComponent<UITransform>(child);
			transform.m_X.m_Constraint = 0;
			transform.m_X.m_Value = -pComponent.m_ScrollPosition.x;
			transform.m_Y.m_Constraint = 0;
			transform.m_Y.m_Value = -pComponent.m_ScrollPosition.y;
			pRegistry->SetEntityDirty(child);
			UITransformSystem::CalculateMatrix(pRegistry, child, transform, false);
		}

		pComponent.m_Dirty = false;
	}

	void UIScrollViewSystem::OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent)
	{
		pComponent.m_Dirty = true;
	}
}
