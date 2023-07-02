#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "DefaultComponentEditor.h"
#include "TransformEditor.h"
#include "PhysicsBodyEditor.h"
#include "ScriptedComponentEditor.h"
#include "CharacterControllerEditor.h"
#include "CreateEntityObjectsCallbacks.h"

#include <EntitySceneScenesModule.h>
#include <EditorPlayer.h>
#include <Components.h>
#include <Reflection.h>
#include <ObjectMenu.h>

#define OBJECT_CREATE_MENU(name, component) std::stringstream name##MenuName; \
name##MenuName << STRINGIFY(Create/Entity Object/) << EntitySceneObjectEditor::GetComponentIcon<component>() << "  " << STRINGIFY(name); \
ObjectMenu::AddMenuItem(name##MenuName.str(), Create##name, T_SceneObject | T_Scene | T_Hierarchy);

namespace Glory::Editor
{
	const std::vector<std::type_index> EntityScenesEditorExtension::m_ComponentsToUpdateInEditor =
	{
		typeid(Transform),
		typeid(CameraComponent),
		typeid(LookAt),
	};

	EntityScenesEditorExtension::EntityScenesEditorExtension()
	{
	}

	EntityScenesEditorExtension::~EntityScenesEditorExtension()
	{
	}

	void EntityScenesEditorExtension::RegisterEditors()
	{
		Editor::RegisterEditor<EntitySceneObjectEditor>();
		Editor::RegisterEditor<DefaultComponentEditor>();
		Editor::RegisterEditor<TransformEditor>();
		Editor::RegisterEditor<ScriptedComponentEditor>();
		Editor::RegisterEditor<PhysicsBodyEditor>();
		Editor::RegisterEditor<CharacterControllerEditor>();

		EditorPlayer::RegisterLoopHandler(this);

		PropertyDrawer::RegisterPropertyDrawer<SimplePropertyDrawerTemplate<MeshMaterial>>();

		OBJECT_CREATE_MENU(Mesh, MeshRenderer);
		OBJECT_CREATE_MENU(Model, ModelRenderer);
		OBJECT_CREATE_MENU(Camera, CameraComponent);
		OBJECT_CREATE_MENU(Light, LightComponent);
		OBJECT_CREATE_MENU(Scripted, ScriptedComponent);
		OBJECT_CREATE_MENU(PhysicsBody, PhysicsBody);
		OBJECT_CREATE_MENU(Character, CharacterController);
	}

	const char* EntityScenesEditorExtension::ModuleName()
	{
		return "Entity Scenes";
	}

	void EntityScenesEditorExtension::HandleBeforeStart(Module* pModule)
	{
	}

	void EntityScenesEditorExtension::HandleStart(Module* pModule)
	{
		EntitySceneScenesModule* pScenesModule = (EntitySceneScenesModule*)pModule;
		Glory::Utils::ECS::ComponentTypes* pComponentTypes = pScenesModule->ComponentTypesInstance();
		Glory::Utils::ECS::ComponentTypes::SetInstance(pComponentTypes);

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			EntityRegistry* pRegistry = pEntityScene->GetRegistry();
			pRegistry->InvokeAll(Glory::Utils::ECS::InvocationType::Start);
		}
	}

	void EntityScenesEditorExtension::HandleStop(Module* pModule)
	{
		EntitySceneScenesModule* pScenesModule = (EntitySceneScenesModule*)pModule;
		Glory::Utils::ECS::ComponentTypes* pComponentTypes = pScenesModule->ComponentTypesInstance();
		Glory::Utils::ECS::ComponentTypes::SetInstance(pComponentTypes);

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			EntityRegistry* pRegistry = pEntityScene->GetRegistry();
			pRegistry->InvokeAll(Glory::Utils::ECS::InvocationType::Stop);
		}
	}

	void EntityScenesEditorExtension::HandleUpdate(Module* pModule)
	{
		EntitySceneScenesModule* pScenesModule = (EntitySceneScenesModule*)pModule;
		Glory::Utils::ECS::ComponentTypes* pComponentTypes = pScenesModule->ComponentTypesInstance();
		Glory::Utils::ECS::ComponentTypes::SetInstance(pComponentTypes);

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			EntityRegistry* pRegistry = pEntityScene->GetRegistry();
			for (size_t i = 0; i < m_ComponentsToUpdateInEditor.size(); i++)
			{
				uint32_t hash = ResourceType::GetHash(m_ComponentsToUpdateInEditor[i]);
				pRegistry->InvokeAll(hash, Glory::Utils::ECS::InvocationType::Update);
			}
		}
	}
}

Glory::Editor::BaseEditorExtension* LoadExtension()
{
	return new Glory::Editor::EntityScenesEditorExtension();
}

void SetContext(Glory::GloryContext* pContext, ImGuiContext* pImGUIContext)
{
	Glory::GloryContext::SetContext(pContext);
	ImGui::SetCurrentContext(pImGUIContext);
}
