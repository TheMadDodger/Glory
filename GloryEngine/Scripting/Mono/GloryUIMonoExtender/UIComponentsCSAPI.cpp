#include "UIComponentsCSAPI.h"

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

#define UI UIComponents_EngineInstance->GetOptionalModule<UIRendererModule>()

	uint64_t UIRenderer_GetDocumentID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_Document.AssetUUID();
	}

	void UIRenderer_SetDocumentID(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t documentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_Document.SetUUID(documentID);
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
	}

	Vec2Wrapper UIRenderer_GetResolution(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		return uiComp.m_Resolution;
	}

	void UIRenderer_SetResolution(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec2Wrapper resolution)
	{
		UIRenderer& uiComp = GetComponent<UIRenderer>(sceneID, objectID, componentID);
		uiComp.m_Resolution = ToGLMVec2(resolution);
	}

#pragma endregion

#pragma region Binding

	void UIComponentsCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* UI Renderer */
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetDocumentID", UIRenderer_GetDocumentID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetDocumentID", UIRenderer_SetDocumentID);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetTarget", UIRenderer_GetTarget);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetTarget", UIRenderer_SetTarget);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetResolutionMode", UIRenderer_GetResolutionMode);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetResolutionMode", UIRenderer_SetResolutionMode);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_GetResolution", UIRenderer_GetResolution);
		BIND("GloryEngine.UI.UIRenderer::UIRenderer_SetResolution", UIRenderer_SetResolution);
	}

	void UIComponentsCSAPI::SetEngine(Engine* pEngine)
	{
		UIComponents_EngineInstance = pEngine;
	}

#pragma endregion

}