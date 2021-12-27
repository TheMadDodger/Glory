#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "TransformComponentEditor.h"
#include "CameraComponentEditor.h"
#include "DefaultComponentEditor.h"

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
		Editor::RegisterEditor<DefaultComponentEditor>();
	}
}