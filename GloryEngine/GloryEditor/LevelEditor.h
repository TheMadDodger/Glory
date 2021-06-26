//#pragma once
//#include <GameScene.h>
//
//class EditorWindow;
//
//class LevelEditor : public Spartan::GameScene
//{
//public:
//	LevelEditor();
//	virtual ~LevelEditor();
//
//	void RenderGUI();
//
//private:
//	virtual void Initialize(const Spartan::GameContext &gameContext);
//	virtual void Update(const Spartan::GameContext &gameContext);
//	virtual void Draw(const Spartan::GameContext &gameContext);
//
//private:
//	void CreateObjectList();
//	void ChildrenList(Spartan::GameObject *pChild);
//	void CreateInspector();
//	void ObjectMenu();
//
//private:
//	int m_I = 0;
//	Spartan::GameObject *m_CurrentSelectedObject = nullptr;
//	Spartan::GameObject *m_CurrentRightClickedObject = nullptr;
//
//	std::map<std::string, bool> m_ComponentsCreated;
//	std::vector<EditorWindow*> m_pEditorWindows;
//};
//
