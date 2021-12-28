#pragma once
#include <Editor.h>
#include <EntityComponentObject.h>

namespace Glory::Editor
{
	class DefaultComponentEditor : public EditorTemplate<DefaultComponentEditor, EntityComponentObject>
	{
	public:
		DefaultComponentEditor();
		~DefaultComponentEditor();

	private:
		virtual void OnGUI() override;
	};
}
