#pragma once
#include <EntityComponentEditor.h>
#include <MonoComponents.h>

namespace Glory::Editor
{
	class MonoScriptComponentEditor : public EntityComponentEditor<MonoScriptComponentEditor, MonoScriptComponent>
	{
	public:
		MonoScriptComponentEditor();
		virtual ~MonoScriptComponentEditor();

		virtual void Initialize() override;

	private:
		virtual bool OnGUI() override;
		virtual std::string Name() override;

		size_t m_LastCompilationVersion{ 0 };
	};
}
