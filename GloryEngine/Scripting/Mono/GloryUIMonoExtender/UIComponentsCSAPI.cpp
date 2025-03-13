#include "UIComponentsCSAPI.h"

#include <Components.h>
#include <UIDocument.h>
#include <UIRendererModule.h>
#include <UIComponents.h>
#include <EntityCSAPI.h>
#include <cstdint>
#include <UUID.h>
#include <GScene.h>
#include <SceneManager.h>
#include <MathCSAPI.h>
#include <GloryMonoScipting.h>
#include <Debug.h>
#include <Engine.h>

#define UI_MODULE UIComponents_EngineInstance->GetOptionalModule<UIRendererModule>()
namespace Glory
{
	Engine* UIComponents_EngineInstance;

	template<typename T>
	static T& GetComponent(UUID sceneID, UUID objectID, uint64_t componentID)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();
		uint32_t hash = pEntityView->ComponentType(componentID);
		return pScene->GetRegistry().GetComponent<T>(entity.GetEntityID());
	}

#pragma region UI Renderer

	uint64_t UIRenderer_GetRenderDocumentID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_RenderDocumentID;
	}

	uint64_t UIRenderer_GetDocumentID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_Document.AssetUUID();
	}

	void UIRenderer_SetDocumentID(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t documentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_Document.SetUUID(documentID);
		uiComp.m_IsDirty = true;
	}

	UITarget UIRenderer_GetTarget(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_Target;
	}

	void UIRenderer_SetTarget(uint64_t sceneID, uint64_t objectID, uint64_t componentID, UITarget target)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_Target = target;
		uiComp.m_IsDirty = true;
	}

	ResolutionMode UIRenderer_GetResolutionMode(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_ResolutionMode;
	}

	void UIRenderer_SetResolutionMode(uint64_t sceneID, uint64_t objectID, uint64_t componentID, ResolutionMode mode)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_ResolutionMode = mode;
		uiComp.m_IsDirty = true;
	}

	Vec3Wrapper UIRenderer_GetResolution(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return Vec3Wrapper(uiComp.m_Resolution.x, uiComp.m_Resolution.y, 1.0f);
	}

	void UIRenderer_SetResolution(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper resolution)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_Resolution = ToGLMVec3(resolution);
		uiComp.m_IsDirty = true;
	}

	uint64_t UIRenderer_GetWorldMaterialID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_WorldMaterial.AssetUUID();
	}

	void UIRenderer_SetWorldMaterialID(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t materialID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_WorldMaterial.SetUUID(materialID);
		uiComp.m_IsDirty = true;
	}

	Vec3Wrapper UIRenderer_GetWorldSize(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return Vec3Wrapper(uiComp.m_WorldSize.x, uiComp.m_WorldSize.y, 1.0f);
	}

	void UIRenderer_SetWorldSize(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper size)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_WorldSize = ToGLMVec3(size);
		uiComp.m_IsDirty = true;
	}

	bool UIRenderer_GetInputEnabled(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_InputEnabled;
	}

	void UIRenderer_SetInputEnabled(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool enabled)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_InputEnabled = enabled;
	}

	Vec3Wrapper UIRenderer_GetCursor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return Vec3Wrapper(uiComp.m_CursorPos.x, uiComp.m_CursorPos.y, 1.0f);
	}

	void UIRenderer_SetCursor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper cursor)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_CursorPos = ToGLMVec3(cursor);
	}

	bool UIRenderer_GetCursorDown(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_CursorDown;
	}

	void UIRenderer_SetCursorDown(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool down)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_CursorDown = down;
	}

	Vec3Wrapper UIRenderer_ConvertWorldToLocalPos(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper worldPos)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Transform& transform = entity.GetComponent<Transform>();
		const glm::vec4 localPos = glm::inverse(transform.MatTransform)*glm::vec4(ToGLMVec3(worldPos), 1.0f);
		const glm::vec2 screenNDCPos{ (glm::vec2(localPos)/(uiComp.m_WorldSize/2.0f)) };
		const glm::vec2 screenPos{ (screenNDCPos + 1.0f)*0.5f*uiComp.m_Resolution };
		return Vec3Wrapper(screenPos.x, screenPos.y, 0.0f);
	}

#pragma endregion

#pragma region UI Transform

	Vec3Wrapper UITransform_GetPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		return { float(transform.m_X.m_Value), float(transform.m_Y.m_Value), 0.0f };
	}

	void UITransform_SetPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper pos)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		transform.m_X = pos.x;
		transform.m_Y = pos.y;
		pDocument->Registry().SetEntityDirty(entity, true);
	}

	Vec3Wrapper UITransform_GetSize(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		return { float(transform.m_Width.m_Value), float(transform.m_Height.m_Value), 0.0f };
	}

	void UITransform_SetSize(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper size)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		transform.m_Width = size.x;
		transform.m_Height = size.y;
		pDocument->Registry().SetEntityDirty(entity, true);
	}

	Vec3Wrapper UITransform_GetPivot(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		return Vec3Wrapper{ transform.m_Pivot.x, transform.m_Pivot.y, 0.0f };
	}

	void UITransform_SetPivot(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper pivot)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		transform.m_Pivot.x = pivot.x;
		transform.m_Pivot.y = pivot.y;
		pDocument->Registry().SetEntityDirty(entity, true);
	}

	float UITransform_GetRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		return transform.m_Rotation;
	}

	void UITransform_SetRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float rotation)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		transform.m_Rotation = rotation;
		pDocument->Registry().SetEntityDirty(entity, true);
	}

	Vec3Wrapper UITransform_GetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		return Vec3Wrapper{ transform.m_Scale.x, transform.m_Scale.y, 0.0f };
	}

	void UITransform_SetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper scale)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UITransform& transform = pDocument->Registry().GetComponent<UITransform>(entity);
		transform.m_Scale.x = scale.x;
		transform.m_Scale.y = scale.y;
		pDocument->Registry().SetEntityDirty(entity, true);
	}

#pragma endregion

#pragma region UI text

	uint64_t UIText_GetFont(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return 0;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		return uiText.m_Font.AssetUUID();
	}

	void UIText_SetFont(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t fontID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		uiText.m_Font.SetUUID(fontID);
		uiText.m_Dirty = true;
	}

	MonoString* UIText_GetText(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return nullptr;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		return mono_string_new(mono_domain_get(), uiText.m_Text.data());
	}

	void UIText_SetText(uint64_t sceneID, uint64_t objectID, uint64_t componentID, MonoString* text)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		uiText.m_Text = mono_string_to_utf8(text);
		uiText.m_Dirty = true;
	}

	float UIText_GetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return 0.0f;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		return float(uiText.m_Scale);
	}

	void UIText_SetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float scale)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		uiText.m_Scale = scale;
	}

	Vec4Wrapper UIText_GetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		return ToVec4Wrapper(uiText.m_Color);
	}

	void UIText_SetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec4Wrapper color)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		uiText.m_Color = ToGLMVec4(color);
	}

	Alignment UIText_GetAlignment(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return Alignment::Left;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		return uiText.m_Alignment;
	}

	void UIText_SetAlignment(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Alignment alignment)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIText& uiText = pDocument->Registry().GetComponent<UIText>(entity);
		uiText.m_Alignment = alignment;
	}

#pragma endregion

#pragma region UI Image

	uint64_t UIImage_GetImage(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return 0;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIImage& uiImage = pDocument->Registry().GetComponent<UIImage>(entity);
		return uiImage.m_Image.AssetUUID();
	}

	void UIImage_SetImage(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t imageID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIImage& uiImage = pDocument->Registry().GetComponent<UIImage>(entity);
		uiImage.m_Image.SetUUID(imageID);
	}

	Vec4Wrapper UIImage_GetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIImage& uiImage = pDocument->Registry().GetComponent<UIImage>(entity);
		return ToVec4Wrapper(uiImage.m_Color);
	}

	void UIImage_SetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec4Wrapper color)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIImage& uiImage = pDocument->Registry().GetComponent<UIImage>(entity);
		uiImage.m_Color = ToGLMVec4(color);
	}

#pragma endregion

#pragma region UI Box

	Vec4Wrapper UIBox_GetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return {};
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIBox& uiBox = pDocument->Registry().GetComponent<UIBox>(entity);
		return ToVec4Wrapper(uiBox.m_Color);
	}

	void UIBox_SetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec4Wrapper color)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIBox& uiBox = pDocument->Registry().GetComponent<UIBox>(entity);
		uiBox.m_Color = ToGLMVec4(color);
	}

#pragma endregion

#pragma region UI Interaction

	bool UIInteraction_GetEnabled(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return false;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIInteraction& interaction = pDocument->Registry().GetComponent<UIInteraction>(entity);
		return interaction.m_Enabled;
	}

	void UIInteraction_SetEnabled(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool enabled)
	{
		UIDocument* pDocument = UI_MODULE->FindDocument(sceneID);
		if (!pDocument) return;
		const Utils::ECS::EntityID entity = pDocument->EntityID(objectID);
		UIInteraction& interaction = pDocument->Registry().GetComponent<UIInteraction>(entity);
		interaction.m_Enabled = enabled;
	}

#pragma endregion

#pragma region Binding

	void UIComponentsCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* UI Renderer */
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetRenderDocumentID", UIRenderer_GetRenderDocumentID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetDocumentID", UIRenderer_GetDocumentID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetDocumentID", UIRenderer_SetDocumentID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetTarget", UIRenderer_GetTarget);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetTarget", UIRenderer_SetTarget);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetResolutionMode", UIRenderer_GetResolutionMode);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetResolutionMode", UIRenderer_SetResolutionMode);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetResolution", UIRenderer_GetResolution);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetResolution", UIRenderer_SetResolution);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetWorldMaterialID", UIRenderer_GetWorldMaterialID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetWorldMaterialID", UIRenderer_SetWorldMaterialID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetWorldSize", UIRenderer_GetWorldSize);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetWorldSize", UIRenderer_SetWorldSize);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetInputEnabled", UIRenderer_GetInputEnabled);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetInputEnabled", UIRenderer_SetInputEnabled);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetCursor", UIRenderer_GetCursor);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetCursor", UIRenderer_SetCursor);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetCursorDown", UIRenderer_GetCursorDown);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetCursorDown", UIRenderer_SetCursorDown);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_ConvertWorldToLocalPos", UIRenderer_ConvertWorldToLocalPos);

		/* UI Transform */
		BIND("GloryEngine.UI.UITransform::UITransform_GetPosition", UITransform_GetPosition);
		BIND("GloryEngine.UI.UITransform::UITransform_SetPosition", UITransform_SetPosition);
		BIND("GloryEngine.UI.UITransform::UITransform_GetSize", UITransform_GetSize);
		BIND("GloryEngine.UI.UITransform::UITransform_SetSize", UITransform_SetSize);
		BIND("GloryEngine.UI.UITransform::UITransform_GetPivot", UITransform_GetPivot);
		BIND("GloryEngine.UI.UITransform::UITransform_SetPivot", UITransform_SetPivot);
		BIND("GloryEngine.UI.UITransform::UITransform_GetRotation", UITransform_GetRotation);
		BIND("GloryEngine.UI.UITransform::UITransform_SetRotation", UITransform_SetRotation);
		BIND("GloryEngine.UI.UITransform::UITransform_GetScale", UITransform_GetScale);
		BIND("GloryEngine.UI.UITransform::UITransform_SetScale", UITransform_SetScale);

		/* UI Text */
		BIND("GloryEngine.UI.UIText::UIText_GetFont", UIText_GetFont);
		BIND("GloryEngine.UI.UIText::UIText_SetFont", UIText_SetFont);
		BIND("GloryEngine.UI.UIText::UIText_GetText", UIText_GetText);
		BIND("GloryEngine.UI.UIText::UIText_SetText", UIText_SetText);
		BIND("GloryEngine.UI.UIText::UIText_GetScale", UIText_GetScale);
		BIND("GloryEngine.UI.UIText::UIText_SetScale", UIText_SetScale);
		BIND("GloryEngine.UI.UIText::UIText_GetColor", UIText_GetColor);
		BIND("GloryEngine.UI.UIText::UIText_SetColor", UIText_SetColor);
		BIND("GloryEngine.UI.UIText::UIText_GetAlignment", UIText_GetAlignment);
		BIND("GloryEngine.UI.UIText::UIText_SetAlignment", UIText_SetAlignment);

		/* UI Image */
		BIND("GloryEngine.UI.UIImage::UIImage_GetImage", UIImage_GetImage);
		BIND("GloryEngine.UI.UIImage::UIImage_SetImage", UIImage_SetImage);
		BIND("GloryEngine.UI.UIImage::UIImage_GetColor", UIImage_GetColor);
		BIND("GloryEngine.UI.UIImage::UIImage_SetColor", UIImage_SetColor);

		/* UI Box */
		BIND("GloryEngine.UI.UIBox::UIBox_GetColor", UIBox_GetColor);
		BIND("GloryEngine.UI.UIBox::UIBox_SetColor", UIBox_SetColor);

		/* UI Interaction */
		BIND("GloryEngine.UI.UIInteraction::UIInteraction_GetEnabled", UIInteraction_GetEnabled);
		BIND("GloryEngine.UI.UIInteraction::UIInteraction_SetEnabled", UIInteraction_SetEnabled);
	}

	void UIComponentsCSAPI::SetEngine(Engine* pEngine)
	{
		UIComponents_EngineInstance = pEngine;
	}

#pragma endregion

}