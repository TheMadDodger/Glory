#include "EntitySceneObjectEditor.h"
#include "AddComponentAction.h"
#include "RemoveComponentAction.h"
#include "EditorSceneManager.h"
#include "EditorAssetDatabase.h"
#include "FileBrowser.h"

#include <imgui.h>
#include <string>

#include <ObjectManager.h>
#include <SceneObjectNameAction.h>
#include <EnableObjectAction.h>
#include <Undo.h>
#include <algorithm>
#include <EditorUI.h>
#include <string_view>
#include <ResourceType.h>
#include <Components.h>
#include <StringUtils.h>
#include <Reflection.h>
#include <PrefabData.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	std::map<uint32_t, std::string_view> ComponentIcons = {
		{ ResourceTypes::GetHash<Transform>(), ICON_FA_LOCATION_CROSSHAIRS },
		{ ResourceTypes::GetHash<MeshFilter>(), ICON_FA_CUBE },
		{ ResourceTypes::GetHash<MeshRenderer>(), ICON_FA_CUBES },
		{ ResourceTypes::GetHash<ModelRenderer>(), ICON_FA_CUBES },
		{ ResourceTypes::GetHash<CameraComponent>(), ICON_FA_VIDEO },
		{ ResourceTypes::GetHash<LayerComponent>(), ICON_FA_LAYER_GROUP },
		{ ResourceTypes::GetHash<LightComponent>(), ICON_FA_LIGHTBULB },
	};

	EntitySceneObjectEditor::EntitySceneObjectEditor() : m_NameBuff(""), m_Initialized(false), m_AddingComponent(false), m_pObject(nullptr)
	{
	}

	EntitySceneObjectEditor::~EntitySceneObjectEditor()
	{
		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [](Editor* pEditor) { Editor::ReleaseEditor(pEditor); });
		m_pComponentEditors.clear();

		std::for_each(m_pComponents.begin(), m_pComponents.end(), [](EntityComponentObject* pObject) {
			EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Destroy(pObject);
		});
		m_pComponents.clear();
	}

	bool EntitySceneObjectEditor::OnGUI()
	{
		if (!m_Initialized) Initialize();
		m_pObject = (EditableEntity*)m_pTarget;
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_pObject->SceneID());
		Entity entity = pScene->GetEntityByEntityID(m_pObject->EntityID());
		const std::string uuidString = std::to_string(m_pObject->GetUUID());
		ImGui::PushID(uuidString.c_str());
		bool active = entity.IsActiveSelf();
		bool change = false;
		if (EditorUI::CheckBox("Active", &active))
		{
			Undo::StartRecord("Set Active", m_pObject->GetUUID());
			Undo::AddAction(new EnableObjectAction(pScene, active));
			Undo::StopRecord();

			entity.SetActive(active);
			change = true;
		}
		change |= NameGUI();
		ImGui::Spacing();
		change |= ComponentGUI();
		ImGui::PopID();
		return change;
	}

	void EntitySceneObjectEditor::Refresh()
	{
		Initialize();
	}

	std::string_view EntitySceneObjectEditor::GetComponentIcon(uint32_t typeHash)
	{
		auto itor = ComponentIcons.find(typeHash);
		return itor != ComponentIcons.end() ? itor->second : "";
	}

	void EntitySceneObjectEditor::AddComponentIcon(uint32_t hash, std::string_view icon)
	{
		ComponentIcons.emplace(hash, icon);
	}

	void EntitySceneObjectEditor::Initialize()
	{
		std::for_each(m_pComponents.begin(), m_pComponents.end(), [](EntityComponentObject* pObject) { delete pObject; });
		m_pComponents.clear();

		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [](Editor* pEditor) { Editor::ReleaseEditor(pEditor); });
		m_pComponentEditors.clear();

		m_pObject = (EditableEntity*)m_pTarget;
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_pObject->SceneID());
		Entity entity = pScene->GetEntityByEntityID(m_pObject->EntityID());
		const Utils::ECS::EntityID entityID = entity.GetEntityID();
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

		for (size_t i = 0; i < pEntityView->ComponentCount(); i++)
		{
			UUID uuid = pEntityView->ComponentUUIDAt(i);
			uint32_t componentType = pEntityView->ComponentTypeAt(i);
			ObjectManager& objects = EditorApplication::GetInstance()->GetEngine()->GetObjectManager();
			EntityComponentObject* pComponentObject = objects.Find<EntityComponentObject>(uuid);
			if(!pComponentObject)
				pComponentObject = objects.Create<EntityComponentObject>(entityID, uuid, componentType, &entity.GetScene()->GetRegistry());
			m_pComponents.push_back(pComponentObject);
			Editor* pEditor = Editor::CreateEditor(pComponentObject);
			if (pEditor) m_pComponentEditors.push_back(pEditor);
		}

		m_Initialized = true;
	}

	bool EntitySceneObjectEditor::NameGUI()
	{
		ImGui::PushID("Object");

		ImGui::TextDisabled("Entity Object");
		ImGui::Separator();

		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_pObject->SceneID());
		Entity entity = pScene->GetEntityByEntityID(m_pObject->EntityID());
		Utils::ECS::EntityID entityID = entity.GetEntityID();

		const UUID prefabID = pScene->Prefab(m_pObject->GetUUID());
		const UUID childOfPrefabID = pScene->PrefabChild(m_pObject->GetUUID());
		if (prefabID || childOfPrefabID)
		{
			std::string prefabIDString = std::to_string(prefabID ? prefabID : childOfPrefabID);
			ImGui::Text("Linked to prefab:");
			const float textWitdh = ImGui::CalcTextSize(prefabIDString.data()).x;
			ImGui::SameLine();
			const ImVec2 cursorPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos({ cursorPos.x + ImGui::GetContentRegionAvail().x - textWitdh, cursorPos.y });
			ImGui::Text(prefabIDString.data());
		}

		ImGui::BeginDisabled(prefabID || childOfPrefabID);
		const std::string_view originalName = entity.Name();
		const char* name = originalName.data();
		memcpy(m_NameBuff, name, originalName.length() + 1);
		m_NameBuff[originalName.length()] = '\0';

		const UUID uuid = m_pObject->GetUUID();
		std::string uuidString = std::to_string(uuid);
		ImGui::Text("UUID:");
		const float textWitdh = ImGui::CalcTextSize(uuidString.data()).x;
		ImGui::SameLine();
		const ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ cursorPos.x + ImGui::GetContentRegionAvail().x - textWitdh, cursorPos.y });
		ImGui::Text(uuidString.data());
		const bool change = EditorUI::InputText("Name", m_NameBuff, MAXNAMESIZE, ImGuiInputTextFlags_EnterReturnsTrue);
		if (change)
		{
			Undo::StartRecord("Change Name", m_pObject->GetUUID());
			Undo::AddAction(new SceneObjectNameAction(pScene, originalName, m_NameBuff));
			Undo::StopRecord();
			pScene->SetEntityName(entityID, m_NameBuff);
		}
		ImGui::EndDisabled();

		ImGui::PopID();
		return change;
	}

	bool EntitySceneObjectEditor::ComponentGUI()
	{
		bool change = false;

		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_pObject->SceneID());
		Entity entity = pScene->GetEntityByEntityID(m_pObject->EntityID());
		Utils::ECS::EntityID entityID = entity.GetEntityID();
		Glory::Utils::ECS::EntityRegistry& pRegistry = pScene->GetRegistry();

		const bool isPrefab = pScene->Prefab(m_pObject->GetUUID());
		const bool isChildOfPrefab = pScene->PrefabChild(m_pObject->GetUUID());
		ImGui::BeginDisabled(isChildOfPrefab);

		ImGui::PushID("Components");

		ImGui::TextDisabled("Components");
		ImGui::Separator();

		bool removeComponent = false;
		size_t toRemoveComponent = 0;

		int index = 0;
		const std::string& nameString = m_pObject->Name();
		std::for_each(m_pComponentEditors.begin(), m_pComponentEditors.end(), [&](Editor* pEditor)
		{
			std::string_view icon = "";
			const uint32_t componentHash = m_pComponents[index]->ComponentType();
			if(ComponentIcons.find(componentHash) != ComponentIcons.end())
				icon = ComponentIcons.at(componentHash);

			ImGui::PushID(index);

			/* Do not disable editing the transform */
			ImGui::BeginDisabled(isPrefab && index);

			std::string label = std::string(icon) + "	" + pEditor->Name();
			const bool open = EditorUI::Header(label);

			if (ImGui::IsItemClicked(1))
			{
				ImGui::OpenPopup("ComponentRightClick");
			}

			if(open) change |= pEditor->OnGUI();

			if (ImGui::BeginPopup("ComponentRightClick"))
			{
				const bool removeAllowed = index != 0;
				if (ImGui::MenuItem("Remove", "", false, removeAllowed))
				{
					removeComponent = true;
					toRemoveComponent = index;
				}
				if (!removeAllowed && ImGui::IsItemHovered())
					ImGui::SetTooltip("You cannot remove the Transform of an entity");

				ImGui::EndPopup();
			}

			ImGui::EndDisabled();

			ImGui::PopID();

			ImGui::Spacing();

			++index;
		});
		ImGui::PopID();

		ImGui::Separator();
		const float buttonWidth = ImGui::GetContentRegionAvail().x;
		ImGui::BeginDisabled(isPrefab);
		if (ImGui::Button("Add Component", { buttonWidth, 0.0f }))
		{
			EntityComponentPopup::Open(entityID, &pRegistry);
			m_AddingComponent = true;
		}
		ImGui::EndDisabled();

		if (removeComponent)
		{
			Undo::StartRecord("Remove Component", m_pTarget->GetUUID());
			Undo::AddAction(new RemoveComponentAction(&pRegistry, entityID, toRemoveComponent));
			pRegistry.RemoveComponentAt(entityID, toRemoveComponent);
			Undo::StopRecord();

			Initialize();
			change = true;
		}

		if (m_AddingComponent)
		{
			uint32_t toAddTypeHash = EntityComponentPopup::GetLastSelectedComponentTypeHash();
			if (toAddTypeHash)
			{
				Undo::StartRecord("Add Component", m_pTarget->GetUUID());
				UUID uuid = UUID();
				size_t index = m_pComponentEditors.size();
				pRegistry.CreateComponent(entityID, toAddTypeHash, uuid);
				Undo::AddAction(new AddComponentAction(toAddTypeHash, uuid, index));
				Undo::StopRecord();

				m_AddingComponent = false;
				Initialize();
				change = true;
			}
		}

		m_ComponentPopup.OnGUI();

		if (change) EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
		ImGui::EndDisabled();
		return change;
	}

	void EntitySceneObjectEditor::DrawObjectNodeName(Entity& entity, bool isPrefab)
	{
		Utils::ECS::EntityView* pEntityView = entity.GetScene()->GetRegistry().GetEntityView(entity.GetEntityID());
		std::stringstream stream;
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			const uint32_t typeHash = pEntityView->ComponentTypeAt(i);
			stream << ComponentIcons.at(typeHash) << ' ';
		}

		const std::string componentLabels = stream.str();
		ImGui::TextColored(isPrefab ? ImVec4{0.5f, 0.5f, 1.0f, 1.0f} : ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f },
			" %s %s", entity.IsHierarchyActive() ? ICON_FA_EYE : ICON_FA_EYE_SLASH, entity.Name().data());

		const float compLabelsWidth = ImGui::CalcTextSize(componentLabels.data()).x;
		const float availableWidth = ImGui::GetWindowContentRegionWidth() - ImGui::GetWindowPos().x;
		ImGui::SameLine(availableWidth - compLabelsWidth);
		ImGui::TextUnformatted(componentLabels.data());
	}

	bool EntitySceneObjectEditor::SearchCompare(std::string_view search, Entity& entity)
	{
		Utils::ECS::EntityView* pEntityView = entity.GetScene()->GetRegistry().GetEntityView(entity.GetEntityID());

		if (search.size() > 2 && search[1] == ':')
		{
			const char searchType = search[0];
			switch (searchType)
			{
			case 'C':
			case 'c':
			{
				const std::string_view comp = search.substr(2);
				for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
				{
					const uint32_t typeHash = pEntityView->ComponentTypeAt(i);
					const TypeData* pType = Utils::Reflect::Reflect::GetTyeData(typeHash);
					const std::string_view name{pType->TypeName()};
					if (Utils::CaseInsensitiveSearch(name, comp) == std::string::npos) continue;
					return true;
				}
				return false;
				break;
			}
			default:
				break;
			}
		}

		return Utils::CaseInsensitiveSearch(entity.Name(), search) != std::string::npos;
	}

	void EntitySceneObjectEditor::ConvertToPrefabMenuItem(Object* pObject, const ObjectMenuType&)
	{
		EditableEntity* pSceneObject = (EditableEntity*)pObject;
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
		Entity entity = pScene->GetEntityByEntityID(pSceneObject->EntityID());
		const std::filesystem::path path = FileBrowser::GetCurrentPath();
		ConvertToPrefab(entity, path);
	}

	void EntitySceneObjectEditor::ConvertToPrefab(Entity& entity, std::filesystem::path path)
	{
		GScene* pScene = entity.GetScene();
		PrefabData* pPrefab = PrefabData::CreateFromEntity(pScene, entity.GetEntityID());
		path.append(entity.Name()).replace_extension(".gentity");
		const UUID prefabUUID = EditorAssetDatabase::CreateAsset(pPrefab, path.string());
		pScene->SetPrefab(entity.GetEntityID(), prefabUUID);
		EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
	}

	void EntitySceneObjectEditor::UnpackPrefabMenuItem(Object* pObject, const ObjectMenuType&)
	{
		EditableEntity* pSceneObject = (EditableEntity*)pObject;
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pSceneObject->SceneID());
		Entity entity = pScene->GetEntityByEntityID(pSceneObject->EntityID());
		if (!pScene->Prefab(pSceneObject->GetUUID())) return;
		pScene->UnsetPrefab(entity.GetEntityID());
		EditorApplication::GetInstance()->GetSceneManager().SetSceneDirty(pScene);
	}
}
