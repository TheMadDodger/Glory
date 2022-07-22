#include "EntityScenesEditorExtension.h"
#include "EntitySceneObjectEditor.h"
#include "DefaultComponentEditor.h"
#include "TransformEditor.h"

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
		Editor::RegisterEditor<TransformEditor>();
	}
}

GLORY_API void LoadExtension(Glory::Editor::EditorContext* pContext, std::vector<Glory::Editor::BaseEditorExtension*>& pExtensions)
{
	Glory::Editor::EditorContext::SetContext(pContext);
	pExtensions.push_back(new Glory::Editor::EntityScenesEditorExtension());
}
