#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "DefaultComponentEditor.h"
#include "TransformEditor.h"
#include "PhysicsBodyEditor.h"
#include "ScriptedComponentEditor.h"
#include "CharacterControllerEditor.h"

#include <EntitySceneScenesModule.h>
#include <EditorPlayer.h>
#include <Components.h>
#include <Reflection.h>

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
		GloryECS::ComponentTypes* pComponentTypes = pScenesModule->ComponentTypesInstance();
		GloryECS::ComponentTypes::SetInstance(pComponentTypes);

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			EntityRegistry* pRegistry = pEntityScene->GetRegistry();
			pRegistry->InvokeAll(GloryECS::InvocationType::Start);
		}
	}

	void EntityScenesEditorExtension::HandleStop(Module* pModule)
	{
		EntitySceneScenesModule* pScenesModule = (EntitySceneScenesModule*)pModule;
		GloryECS::ComponentTypes* pComponentTypes = pScenesModule->ComponentTypesInstance();
		GloryECS::ComponentTypes::SetInstance(pComponentTypes);

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			EntityRegistry* pRegistry = pEntityScene->GetRegistry();
			pRegistry->InvokeAll(GloryECS::InvocationType::Stop);
		}
	}

	void EntityScenesEditorExtension::HandleUpdate(Module* pModule)
	{
		EntitySceneScenesModule* pScenesModule = (EntitySceneScenesModule*)pModule;
		GloryECS::ComponentTypes* pComponentTypes = pScenesModule->ComponentTypesInstance();
		GloryECS::ComponentTypes::SetInstance(pComponentTypes);

		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			EntityRegistry* pRegistry = pEntityScene->GetRegistry();
			for (size_t i = 0; i < m_ComponentsToUpdateInEditor.size(); i++)
			{
				uint32_t hash = ResourceType::GetHash(m_ComponentsToUpdateInEditor[i]);
				pRegistry->InvokeAll(hash, GloryECS::InvocationType::Update);
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
