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
		virtual bool OnGUI() override;
		virtual void Initialize() override;
		virtual std::string Name() override;

	private:
		std::string m_Name;
	};
}
