#pragma once
#include <ScenesModule.h>
#include "EditorWindow.h"

namespace Glory::Editor
{
	class SceneGraphWindow : public EditorWindowTemplate<SceneGraphWindow>
	{
	public:
		SceneGraphWindow();
		virtual ~SceneGraphWindow();

	private:
		virtual void OnGUI() override;

		void SceneDropdown(size_t index, GScene* pScene, bool isActive);
		void ChildrenList(size_t index, SceneObject* pScene);

	private:
		struct DNDPayload
		{
			SceneObject* pObject;
		};
	};
}