#include "UIRenderManager.h"
#include "UIRendererModule.h"
#include "UIComponents.h"
#include "UIDocument.h"

#include <GScene.h>
#include <InputModule.h>
#include <Components.h>

namespace Glory
{
	UIRenderManager::UIRenderManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pModule(nullptr), m_pResources(nullptr),
		m_pDebug(nullptr), m_pInput(nullptr), m_pLayers(nullptr)
	{
	}

	UIRenderManager::~UIRenderManager()
	{
	}

	void UIRenderManager::OnStartImpl(Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		if (!pComponent.m_RenderDocumentID) return;
		UIDocument* pDocument = m_pModule->FindDocument(pComponent.m_RenderDocumentID);
		if (!pDocument) return;
		pDocument->Start();
	}

	void UIRenderManager::OnValidateImpl(Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		UIDocumentData* pDocument = pComponent.m_Document.Get(m_pResources);
		if (!pDocument) return;

		Transform& transform = m_pRegistry->GetComponent<Transform>(entity);
		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.GetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_SceneID = pScene->GetUUID();
		data.m_TargetCamera = 0;
		data.m_WorldTransform = transform.MatTransform;
		data.m_Target = pComponent.m_Target;
		data.m_InputEnabled = pComponent.m_InputEnabled;
		pComponent.m_RenderDocumentID = data.m_ObjectID;
		pComponent.m_IsDirty = true;
		data.m_WorldDirty = pComponent.m_IsDirty;
		data.m_ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

		glm::uvec2 resolution = pComponent.m_Resolution;

		switch (pComponent.m_Target)
		{
		case UITarget::CameraOverlay:
		{
			if (!m_pRegistry->HasComponent<CameraComponent>(entity))
			{
				m_pDebug->LogError("Can't overlay UI on camera, entity has no camera component");
				return;
			}
			CameraComponent& camera = m_pRegistry->GetComponent<CameraComponent>(entity);
			data.m_TargetCamera = camera.m_Camera.GetUUID();
			resolution = camera.m_Camera.GetResolution();
			break;
		}
		default:
			break;
		}

		switch (pComponent.m_ResolutionMode)
		{
		case ResolutionMode::CameraScale:
			if (pComponent.m_Target != UITarget::CameraOverlay)
			{
				m_pDebug->LogError("Target not set to CameraOverlay, can't base resolution on camera");
				return;
			}
			data.m_Resolution = glm::uvec2(resolution.x*pComponent.m_Resolution.x, resolution.y*pComponent.m_Resolution.y);
			break;
		case ResolutionMode::Fixed:
			data.m_Resolution = resolution;
			break;
		default:
			break;
		}

		m_pModule->Create(data, pDocument);
	}

	void UIRenderManager::OnDrawImpl(Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		Transform& transform = m_pRegistry->GetComponent<Transform>(entity);
		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.GetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_SceneID = pScene->GetUUID();
		data.m_TargetCamera = 0;
		data.m_WorldTransform = transform.MatTransform;
		data.m_Target = pComponent.m_Target;
		data.m_MaterialID = pComponent.m_WorldMaterial.GetUUID();
		data.m_WorldSize = pComponent.m_WorldSize;
		data.m_WorldDirty = pComponent.m_IsDirty;
		data.m_InputEnabled = pComponent.m_InputEnabled;
		pComponent.m_RenderDocumentID = data.m_ObjectID;
		pComponent.m_IsDirty = false;

		glm::uvec2 resolution = pComponent.m_Resolution;

		switch (pComponent.m_Target)
		{
		case UITarget::CameraOverlay:
		{
			if (!m_pRegistry->HasComponent<CameraComponent>(entity))
				return;
			CameraComponent& camera = m_pRegistry->GetComponent<CameraComponent>(entity);
			data.m_TargetCamera = camera.m_Camera.GetUUID();
			resolution = camera.m_Camera.GetResolution();

			if (m_pInput)
			{
				const glm::vec2 screenScale = 1.0f / m_pInput->GetScreenScale();
				pComponent.m_CursorPos = m_pInput->GetCursorPos(0)*screenScale;
				pComponent.m_CursorScrollDelta = m_pInput->GetCursorScrollDelta(0);
				pComponent.m_CursorDown = m_pInput->IsCursorDown(0);
			}
			break;
		}
		case UITarget::WorldSpaceQuad:
		{
			LayerMask mask;
			if (m_pRegistry->HasComponent<LayerComponent>(entity))
			{
				LayerComponent& layer = m_pRegistry->GetComponent<LayerComponent>(entity);
				mask = layer.m_Layer.Layer(m_pLayers) != nullptr ? layer.m_Layer.Layer(m_pLayers)->m_Mask : UUID(0ull);
			}
			data.m_LayerMask = mask;
			break;
		}
		default:
			break;
		}

		switch (pComponent.m_ResolutionMode)
		{
		case ResolutionMode::CameraScale:
			if (pComponent.m_Target != UITarget::CameraOverlay)
				return;
			data.m_Resolution = glm::uvec2(resolution.x*pComponent.m_Resolution.x, resolution.y*pComponent.m_Resolution.y);
			break;
		case ResolutionMode::Fixed:
			data.m_Resolution = resolution;
			break;
		default:
			break;
		}

		data.m_CursorPos = pComponent.m_CursorPos;
		data.m_CursorScrollDelta = pComponent.m_CursorScrollDelta;
		data.m_CursorDown = pComponent.m_CursorDown;

		m_pModule->Submit(std::move(data));
	}

	void UIRenderManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const UIRenderer& meshRenderer = GetAt(i);
			const UUID document = meshRenderer.m_Document.GetUUID();
			const UUID material = meshRenderer.m_WorldMaterial.GetUUID();
			if (document) references.push_back(document);
			if (material) references.push_back(material);
		}
	}

	void UIRenderManager::OnDeserialize(Utils::BinaryStream&)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const UIRenderer& renderer = GetAt(i);
			renderer.m_Document.ManualRegisterReference();
			renderer.m_WorldMaterial.ManualRegisterReference();
		}
	}

	void UIRenderManager::OnCopy(UIRenderer& renderer)
	{
		renderer.m_Document.ManualRegisterReference();
		renderer.m_WorldMaterial.ManualRegisterReference();
	}

	void UIRenderManager::OnInitialize()
	{
		Bind(DoStart, &UIRenderManager::OnStartImpl);
		Bind(DoValidate, &UIRenderManager::OnValidateImpl);
		Bind(DoDraw, &UIRenderManager::OnDrawImpl);
		Bind(DoGetReferences, &UIRenderManager::GetReferencesImpl);
	}
}
