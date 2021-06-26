#pragma once
#include "EditorWindow.h"
//#include <GameObject.h>
//#include <GameScene.h>

namespace Glory::Editor
{
	class SceneGraphWindow : public EditorWindowTemplate<SceneGraphWindow>
	{
	public:
		SceneGraphWindow();
		virtual ~SceneGraphWindow();

	private:
		virtual void OnGUI() override;

		//void SceneDropdown(GameScene* pScene, bool isActive);
		//void ChildrenList(Spartan::GameObject* pChild);
		//void ObjectMenu();

	private:
		int m_I;
		//GameObject* m_CurrentRightClickedObject;
	};
}