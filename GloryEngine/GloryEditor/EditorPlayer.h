#pragma once
#include <GScene.h>
#include <vector>

namespace Glory::Editor
{
	class EditorPlayer
	{
	private:
		void Start();
		void Stop();

	private:
		friend class EditorApplication;
		EditorPlayer();
		virtual ~EditorPlayer();

		std::string m_SerializedScenes;
		size_t m_UndoHistoryIndex;
		UUID m_SelectedObjectBeforeStart;
	};
}
