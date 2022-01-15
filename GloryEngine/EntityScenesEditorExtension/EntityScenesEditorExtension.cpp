#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "TransformComponentEditor.h"
#include "CameraComponentEditor.h"
#include "LayerComponentEditor.h"
#include "DefaultComponentEditor.h"
#include "MeshRendererComponentEditor.h"


#include "EntitySceneScenesModule.h"

#include <Game.h>
#include <Engine.h>
#include <Selection.h>

namespace Glory::Editor
{
	EntityScenesEditorExtension::EntityScenesEditorExtension()
	{
	}

	EntityScenesEditorExtension::~EntityScenesEditorExtension()
	{
	}

	void EntityScenesEditorExtension::RegisterEditors()
	{
		Editor::RegisterEditor<EntitySceneObjectEditor>();
		Editor::RegisterEditor<TransformComponentEditor>();
		Editor::RegisterEditor<CameraComponentEditor>();
		Editor::RegisterEditor<LayerComponentEditor>();
		Editor::RegisterEditor<MeshRendererComponentEditor>();
		Editor::RegisterEditor<DefaultComponentEditor>();

		Selection::SetActiveObject(((EntitySceneScenesModule*)Game::GetGame().GetEngine()->GetScenesModule())->m_pMaterialData);
	}
}