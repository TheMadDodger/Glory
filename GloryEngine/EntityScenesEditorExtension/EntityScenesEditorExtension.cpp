#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "TransformComponentEditor.h"
#include "CameraComponentEditor.h"
#include "LayerComponentEditor.h"
#include "DefaultComponentEditor.h"
#include "MeshRendererComponentEditor.h"
#include "LightComponentEditor.h"

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
		Editor::RegisterEditor<DefaultComponentEditor>();
		//Editor::RegisterEditor<TransformComponentEditor>();
		//Editor::RegisterEditor<CameraComponentEditor>();
		//Editor::RegisterEditor<LayerComponentEditor>();
		//Editor::RegisterEditor<MeshRendererComponentEditor>();
		//Editor::RegisterEditor<LightComponentEditor>();
	}
}