#include "EditorPlayer.h"
#include "EditorSceneManager.h"
#include "Selection.h"
#include "Undo.h"
#include "SelectAction.h"
#include "DeselectAction.h"

namespace Glory::Editor
{
	void EditorPlayer::Start()
	{
		Object* pSelected = Selection::GetActiveObject();
		if (pSelected != nullptr) m_SelectedObjectBeforeStart = pSelected->GetUUID();
		Selection::Clear();
		m_UndoHistoryIndex = Undo::GetHistorySize();
		YAML::Emitter out;
		EditorSceneManager::SerializeOpenScenes(out);
		m_SerializedScenes = out.c_str();

		if (pSelected) Selection::SetActiveObject(pSelected);
	}

	void EditorPlayer::Stop()
	{
		//Object* pSelected = Selection::GetActiveObject();
		//UUID toSelect = 0;
		//if (pSelected != nullptr) toSelect = pSelected->GetUUID();

		Selection::Clear();
		EditorSceneManager::CloseAll();
		YAML::Node node = YAML::Load(m_SerializedScenes);
		EditorSceneManager::OpenAllFromNode(node);

		//pSelected = Object::FindObject(toSelect);
		//Selection::SetActiveObject(pSelected);
		Undo::ClearHistoryFrom(m_UndoHistoryIndex);

		//toSelect = 0;
		//if (pSelected != nullptr) toSelect = pSelected->GetUUID();
		//if (m_SelectedObjectBeforeStart && m_SelectedObjectBeforeStart != toSelect)
		//{
		//	Undo::StartRecord("Selection");
		//	Undo::AddAction(new DeselectAction(m_SelectedObjectBeforeStart));
		//	if (toSelect) Undo::AddAction(new SelectAction(toSelect));
		//	Undo::StopRecord();
		//}
		//m_SelectedObjectBeforeStart = 0;
	}

	EditorPlayer::EditorPlayer() : m_SerializedScenes(""), m_UndoHistoryIndex(0)
	{
	}

	EditorPlayer::~EditorPlayer()
	{
	}
}
