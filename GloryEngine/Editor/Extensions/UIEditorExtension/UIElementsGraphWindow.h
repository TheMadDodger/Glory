#pragma once
#include <EditorWindow.h>

#include <EntityID.h>

namespace Glory
{
	class UIDocument;
}

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory::Editor
{
	class UIMainWindow;

	class UIElementsGraphWindow : public EditorWindowTemplate<UIElementsGraphWindow>
	{
	public:
		UIElementsGraphWindow();
		virtual ~UIElementsGraphWindow();

	private:
		virtual void OnGUI() override;
		UIMainWindow* GetMainWindow();
		bool ChildrenList(bool first, UUID& selectedEntity, Utils::ECS::EntityID entity, UIDocument* pDocument);
	};
}
