#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "DefaultComponentEditor.h"
#include "TransformEditor.h"
#include <EntitySceneScenesModule.h>
#include <EditorPlayer.h>
#include <Components.h>

//#include <Reflection.h>

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
		//GloryReflect::Reflect::RegisterType<Transform>();

		Editor::RegisterEditor<EntitySceneObjectEditor>();
		Editor::RegisterEditor<DefaultComponentEditor>();
		Editor::RegisterEditor<TransformEditor>();

		EditorPlayer::RegisterLoopHandler(this);
	}

	const const char* EntityScenesEditorExtension::ModuleName()
	{
		return "Entity Scenes";
	}

	void EntityScenesEditorExtension::HandleUpdate(Module* pModule)
	{
		EntitySceneScenesModule* pScenesModule = (EntitySceneScenesModule*)pModule;
		for (size_t i = 0; i < pScenesModule->OpenScenesCount(); i++)
		{
			GScene* pScene = pScenesModule->GetOpenScene(i);
			EntityScene* pEntityScene = (EntityScene*)pScene;
			Registry* pRegistry = pEntityScene->GetRegistry();

			EntitySystems* pSystems = pRegistry->GetSystems();

			for (size_t i = 0; i < pSystems->SystemCount(); i++)
			{
				EntitySystem* pSystem = pSystems->GetSystem(i);
				std::type_index componentType = pSystem->GetComponentType();
				auto it = std::find(m_ComponentsToUpdateInEditor.begin(), m_ComponentsToUpdateInEditor.end(), componentType);
				if (it == m_ComponentsToUpdateInEditor.end()) continue;

				pRegistry->ForEach(componentType,
					[pSystem](Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData)
					{
						pSystem->Update(pRegisrty, entity, pComponentData);
					});
			}
		}
	}
}

GLORY_API Glory::Editor::BaseEditorExtension* LoadExtension()
{
	return new Glory::Editor::EntityScenesEditorExtension();
}

GLORY_API void SetContext(Glory::GloryContext* pContext, ImGuiContext* pImGUIContext)
{
	Glory::GloryContext::SetContext(pContext);
	ImGui::SetCurrentContext(pImGUIContext);
}
