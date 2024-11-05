#include "SceneWindow.h"
#include "SceneViewCamera.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "EditorRenderImpl.h"
#include "Gizmos.h"
#include "Selection.h"
#include "EditableEntity.h"
#include "EntityEditor.h"
#include "EditorAssetDatabase.h"

#include <CameraManager.h>
#include <SceneManager.h>
#include <AssetManager.h>
#include <PrefabData.h>
#include <Engine.h>
#include <RendererModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_internal.h>
#include <EditorUI.h>
#include <Shortcuts.h>
#include <Dispatcher.h>
#include <ImGuiHelpers.h>
#include <Components.h>

namespace Glory::Editor
{
	static const char* Shortcut_View_Perspective = "Switch To Perspective";
	static const char* Shortcut_View_Orthographic = "Switch To Orthographic";

	SceneWindow::SceneWindow()
		: EditorWindowTemplate("Scene", 1280.0f, 720.0f),
		m_DrawGrid(true), m_SelectedRenderTextureIndex(-1), m_SelectedFrameBufferIndex(0),
		m_ViewEventID(0)
	{
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
		m_pPreviewScene = new GScene("Preview");
		m_pPreviewScene->Settings().m_RenderLate = true;
	}

	SceneWindow::~SceneWindow()
	{
		delete m_pPreviewScene;
	}

	void SceneWindow::OnOpen()
	{
		m_SelectedFrameBufferIndex = 0;

		ImGuiIO& io = ImGui::GetIO();

		m_SceneCamera.Initialize();
		m_SceneCamera.m_Width = (uint32_t)m_WindowDimensions.x;
		m_SceneCamera.m_Height = (uint32_t)m_WindowDimensions.x;
		m_SceneCamera.SetPerspective((uint32_t)m_WindowDimensions.x, (uint32_t)m_WindowDimensions.y, 60.0f, 0.1f, 3000.0f);
		m_SceneCamera.m_Camera.EnableOutput(true);

		m_ViewEventID = GetViewEventDispatcher().AddListener([&](const ViewEvent& e) {
			m_SceneCamera.m_IsOrthographic = e.Ortho;
			m_SceneCamera.UpdateCamera();
		});

		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		pScenes->AddExternalScene(m_pPreviewScene);
	}

	void SceneWindow::OnClose()
	{
		Gizmos::Clear();
		m_SceneCamera.Cleanup();

		GetViewEventDispatcher().RemoveListener(m_ViewEventID);

		SceneManager* pScenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		pScenes->RemoveExternalScene(m_pPreviewScene);
	}

	Dispatcher<ViewEvent>& SceneWindow::GetViewEventDispatcher()
	{
		static Dispatcher<ViewEvent> dispatcher;
		return dispatcher;
	}

	void SceneWindow::OnGUI()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		RenderTexture* pRenderTexture = pEngine->GetCameraManager().GetRenderTextureForCamera(m_SceneCamera.m_Camera, pEngine, 0, false);

		MenuBar();
		CameraUpdate();
		DrawScene();
	}

	void SceneWindow::Draw()
	{
		EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>()->Submit(m_SceneCamera.m_Camera);
		EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>()->Submit(m_PickPos, m_SceneCamera.m_Camera.GetUUID());
	}

	void SceneWindow::MenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Grid", NULL, m_DrawGrid))
			{
				m_DrawGrid = !m_DrawGrid;
			}

			if (ImGui::BeginMenu(m_SelectedRenderTextureIndex == -1 ? "Final" :
				m_SceneCamera.m_Camera.GetRenderTexture(size_t(m_SelectedRenderTextureIndex))->AttachmentName(m_SelectedFrameBufferIndex).c_str()))
			{
				if (ImGui::MenuItem("Final", NULL, m_SelectedRenderTextureIndex == -1))
				{
					m_SelectedRenderTextureIndex = -1;
					m_SelectedFrameBufferIndex = 0;
				}

				for (size_t i = 0; i < m_SceneCamera.m_Camera.GetRenderTextureCount(); ++i)
				{
					RenderTexture* pRenderTexture = m_SceneCamera.m_Camera.GetRenderTexture(i);
					for (size_t j = 0; j < pRenderTexture->AttachmentCount(); ++j)
					{
						const std::string& name = pRenderTexture->AttachmentName(j);
						if (ImGui::MenuItem(name.c_str(), NULL, m_SelectedRenderTextureIndex == i && m_SelectedFrameBufferIndex == j))
						{
							m_SelectedRenderTextureIndex = i;
							m_SelectedFrameBufferIndex = j;
						}
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Perspective", Shortcuts::GetShortcutString(Shortcut_View_Perspective).data(), !m_SceneCamera.m_IsOrthographic))
				{
					m_SceneCamera.m_IsOrthographic = false;
					m_SceneCamera.UpdateCamera();
				}
				if (ImGui::MenuItem("Orthographic", Shortcuts::GetShortcutString(Shortcut_View_Orthographic).data(), m_SceneCamera.m_IsOrthographic))
				{
					m_SceneCamera.m_IsOrthographic = true;
					m_SceneCamera.UpdateCamera();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Camera"))
			{
				ImGui::TextUnformatted("				Camera Settings					");
				ImGui::Separator();
				bool change = false;
				change |= EditorUI::InputFloat("FOV", &m_SceneCamera.m_HalfFOV, 0, 175.0f);
				change |= EditorUI::InputFloat("Near", &m_SceneCamera.m_Near, 0.001f);
				change |= EditorUI::InputFloat("Far", &m_SceneCamera.m_Far, m_SceneCamera.m_Near + 1.0f);
				EditorUI::InputFloat("Fly Speed", &m_SceneCamera.m_MovementSpeed, 0.001f);
				EditorUI::InputFloat("Sensitivity", &m_SceneCamera.m_FreeLookSensitivity, 0.001f);
				EditorUI::InputFloat("Zoom Sensitivity", &m_SceneCamera.m_ZoomSensitivity, 0.001f);
				if (change) m_SceneCamera.UpdateCamera();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void SceneWindow::CameraUpdate()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max)) m_SceneCamera.Update();
		m_SceneCamera.m_Width = (uint32_t)m_WindowDimensions.x;
		m_SceneCamera.m_Height = (uint32_t)m_WindowDimensions.x;
		if (!m_SceneCamera.SetResolution((uint32_t)m_WindowDimensions.x, (uint32_t)m_WindowDimensions.y)) return;
		m_SceneCamera.UpdateCamera();
	}

	void SceneWindow::DrawScene()
	{
		RenderTexture* pSceneTexture = m_SceneCamera.m_Camera.GetOutputTexture();
		if (pSceneTexture == nullptr) return;

		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		ImVec2 screenPos = ImGui::GetCursorScreenPos();
		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		float aspect = (regionAvail.x) / (regionAvail.y);
		float width = regionAvail.x;
		float height = width / aspect;

		const ImVec2 windowPos = ImGui::GetWindowPos();
		const ImVec2 min = screenPos;
		const ImVec2 max = screenPos + ImVec2{ width, height };
		const ImRect rect{ min, max };
		if (ImGui::BeginDragDropTargetCustom(rect, ImGui::GetCurrentWindow()->ID))
		{
			const ImGuiPayload* pPayload = ImGui::GetDragDropPayload();
			if (pPayload && pPayload->IsDataType(STNames[ST_Path]))
			{
				std::string path = (const char*)pPayload->Data;
				HandleDragAndDrop(path);
			}
			ImGui::EndDragDropTarget();
		}

		Texture* pTexture = m_SelectedRenderTextureIndex == -1 ? pSceneTexture->GetTextureAttachment(0) :
			m_SceneCamera.m_Camera.GetRenderTexture(size_t(m_SelectedRenderTextureIndex))->GetTextureAttachment(m_SelectedFrameBufferIndex);

		ImVec2 viewportSize = ImVec2(width, height);
		ImGui::Image(pRenderImpl->GetTextureID(pTexture), viewportSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(screenPos.x, screenPos.y, width, height);

		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		Picking(screenPos, viewportSize);

		if (m_PrefabInstance && !ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
		{
			m_pPreviewScene->DestroyEntity(m_pPreviewScene->GetEntityByUUID(m_PrefabInstance).GetEntityID());
			m_PrefabInstance = 0;
			m_PreviewPrefabID = 0;
		}
		else if (m_PrefabInstance)
		{
			Entity entity = m_pPreviewScene->GetEntityByUUID(m_PrefabInstance);
			Transform& transform = entity.GetComponent<Transform>();
			transform.Position = GetPosition();
			transform.Rotation = GetRotation();
			m_pPreviewScene->GetRegistry().SetEntityDirty(entity.GetEntityID());
		}

		viewManipulateRight = ImGui::GetWindowPos().x + width;
		viewManipulateTop = ImGui::GetWindowPos().y;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		m_WindowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar | (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0);

		const glm::mat4& cameraView = m_SceneCamera.m_Camera.GetView();
		const glm::mat4& cameraProjection = m_SceneCamera.m_Camera.GetProjection();

		glm::mat4 identityMatrix = glm::identity<glm::mat4>();
		if (m_DrawGrid) ImGuizmo::DrawGrid((float*)&cameraView, (float*)&cameraProjection, (float*)&identityMatrix, 100.f);

		//ImGuizmo::DrawCubes((float*)&cameraView, (float*)&cameraProjection, (float*)&identityMatrix, 1);

		Gizmos::DrawGizmos(cameraView, cameraProjection);

		float camDistance = 8.f;
		ImGuizmo::ViewManipulate(m_SceneCamera.m_Camera.GetViewPointer(), camDistance, ImVec2(viewManipulateRight - 256, viewManipulateTop + 64), ImVec2(256, 256), 0x10101010);
	}

	void SceneWindow::Picking(const ImVec2& min, const ImVec2& size)
	{
		ImVec2 viewportMax = ImVec2(min.x + size.x, min.y + size.y);
		ImVec2 coord = ImGui::GetMousePos();
		glm::vec2 viewportCoord = glm::vec2(coord.x - min.x, coord.y - min.y);
		viewportCoord = viewportCoord / glm::vec2(size.x, size.y);

		glm::uvec2 resolution = m_SceneCamera.m_Camera.GetResolution();
		glm::uvec2 textureCoord = viewportCoord * (glm::vec2)resolution;
		textureCoord.y = resolution.y - textureCoord.y;
		m_PickPos = textureCoord;

		if (!m_BlockNextPick && !ImGuizmo::IsOver() && ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0) && ImGui::IsMouseHoveringRect(min, viewportMax))
		{
			Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
			GScene* pScene = pEngine->GetSceneManager()->GetHoveringEntityScene();
			if (!pScene)
			{
				Selection::SetActiveObject(nullptr);
				return;
			}
			const UUID objectID = pEngine->GetSceneManager()->GetHoveringEntityUUID();
			Entity entityHandle = pScene->GetEntityByUUID(objectID);
			if (!entityHandle.IsValid())
			{
				Selection::SetActiveObject(nullptr);
				return;
			}
			Selection::SetActiveObject(GetEditableEntity(entityHandle.GetEntityID(), pScene));
		}
		m_BlockNextPick = false;
	}

	void SceneWindow::HandleDragAndDrop(std::string& path)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		const UUID uuid = EditorAssetDatabase::FindAssetUUID(path);
		if (!uuid) return;
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(uuid, meta)) return;
		ResourceTypes& types = pEngine->GetResourceTypes();
		ResourceType* pResourceType = types.GetResourceType(meta.Hash());

		PrefabData* pPrefab = nullptr;
		if (meta.Hash() != ResourceTypes::GetHash<PrefabData>())
		{
			bool found = false;
			for (size_t i = 0; i < types.SubTypeCount(pResourceType); ++i)
			{
				ResourceType* pSubResourceType = types.GetSubType(pResourceType, i);
				if (!pSubResourceType) continue;
				if (pSubResourceType->Hash() != ResourceTypes::GetHash<PrefabData>()) continue;

				pPrefab = pEngine->GetAssetManager().GetAssetImmediate<PrefabData>(uuid);
				found = true;
				break;
			}
			if (!found)
			{
				const std::filesystem::path subPath = std::filesystem::path(path).filename().replace_extension().concat("_Prefab");
				const UUID prefabID = EditorAssetDatabase::FindAssetUUID(path, subPath);
				if (!prefabID)
					return;
				pPrefab = pEngine->GetAssetManager().GetAssetImmediate<PrefabData>(prefabID);
				if (!pPrefab)
					return;
			}
		}
		else
		{
			pPrefab = pEngine->GetAssetManager().GetAssetImmediate<PrefabData>(uuid);
		}

		if (!pPrefab) return;

		const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(STNames[ST_Path]);
		if (pPayload)
		{
			if (m_PrefabInstance)
			{
				m_pPreviewScene->DestroyEntity(m_pPreviewScene->GetEntityByUUID(m_PrefabInstance).GetEntityID());
				m_PrefabInstance = 0;
			}

			/* Spawn it */
			GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetActiveScene(true);
			const glm::vec3 pos = GetPosition();
			const glm::quat rot = GetRotation();
			Entity entity = pScene->InstantiatePrefab(0, pPrefab, pos, rot, glm::vec3{ 1, 1, 1 });
			EditableEntity* pEntity = GetEditableEntity(entity.GetEntityID(), pScene);
			Selection::SetActiveObject(pEntity);
			m_BlockNextPick = true;
			return;
		}

		if (m_PreviewPrefabID != pPrefab->GetUUID() || m_PrefabInstance == 0)
		{
			if (m_PrefabInstance)
			{
				m_pPreviewScene->DestroyEntity(m_pPreviewScene->GetEntityByUUID(m_PrefabInstance).GetEntityID());
				m_PrefabInstance = 0;
				m_PreviewPrefabID = 0;
			}

			m_PreviewPrefabID = pPrefab->GetUUID();
			Entity entity = m_pPreviewScene->InstantiatePrefab(0, pPrefab, glm::vec3{}, glm::quat{ 1, 0, 0, 0 }, glm::vec3{ 1.0f, 1.0f, 1.0f });
			m_PrefabInstance = entity.EntityUUID();
		}
	}

	const glm::vec3 SceneWindow::GetPosition() const
	{
		const float* snap = Gizmos::GetSnap(ImGuizmo::OPERATION::TRANSLATE);
		SceneManager* scenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		glm::vec3 pos = scenes->GetHoveringPosition();
		const UUID hoveringObject = scenes->GetHoveringEntityUUID();
		const GScene* hoveringScene = scenes->GetHoveringEntityScene();
		if (!hoveringScene || !hoveringObject)
		{
			const glm::uvec2 resolution = m_SceneCamera.m_Camera.GetResolution();
			const glm::vec2 coord = glm::vec2{ m_PickPos.x / (float)resolution.x, m_PickPos.y / (float)resolution.y };

			const glm::vec4 clipSpacePosition{ coord * 2.0f - 1.0f, 0.99f, 1.0f };
			const glm::mat4 projectionInverse = m_SceneCamera.m_Camera.GetProjectionInverse();
			const glm::mat4 viewInverse = m_SceneCamera.m_Camera.GetViewInverse();
			glm::vec4 viewSpacePosition = projectionInverse * clipSpacePosition;

			/* Perspective division */
			viewSpacePosition /= viewSpacePosition.w;
			const glm::vec4 worldSpacePosition = viewInverse * viewSpacePosition;
			pos = worldSpacePosition;
		}

		if (!snap) return pos;
		pos.x = std::round(pos.x / *snap)**snap;
		pos.y = std::round(pos.y / *snap)**snap;
		pos.z = std::round(pos.z / *snap)**snap;
		return pos;
	}

	const glm::quat SceneWindow::GetRotation() const
	{
		SceneManager* scenes = EditorApplication::GetInstance()->GetEngine()->GetSceneManager();
		const UUID hoveringObject = scenes->GetHoveringEntityUUID();
		const GScene* hoveringScene = scenes->GetHoveringEntityScene();
		if (!hoveringScene || !hoveringObject)
		{
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}

		const glm::vec3 normal = scenes->GetHoveringNormal();
		const glm::vec3 forward{ 0.0f, 0.0f, 1.0f };
		const glm::vec3 right{ -1.0f, 0.0f, 0.0f };
		const float forwardDot = std::abs(glm::dot(normal, forward));
		const float rightDot = std::abs(glm::dot(normal, right));
		const glm::vec3 other = glm::cross(forwardDot > rightDot ? right : forward, normal);
		const glm::vec3 actualForward = glm::normalize(glm::cross(other, normal));
		return glm::conjugate(glm::quatLookAt(actualForward, normal));
	}
}
