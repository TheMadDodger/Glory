#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class FSMEditor;

	class StringTableEditor : public EditorWindowTemplate<StringTableEditor>
	{
	public:
		StringTableEditor();
		virtual ~StringTableEditor();

		void SetTable(UUID tableID);

	private:
		virtual void OnGUI() override;
		virtual void Update() override;
		virtual void Draw() override;
		virtual void OnOpen() override;

		bool FolderGUI(Utils::YAMLFileRef& file, Utils::NodeValueRef baseNode, Utils::NodeValueRef node, float rowHeight);
		bool NewItemGUI(Utils::YAMLFileRef& file, Utils::NodeValueRef node, float rowHeight);

		UUID m_TableID;
	};
}