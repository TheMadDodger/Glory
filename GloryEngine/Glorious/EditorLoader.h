#pragma once
#include <GloryEngine.h>
#include <EditorApplication.h>
#include <Game.h>

namespace Glory
{
	class EditorLoader
	{
	public:
		EditorLoader();
		virtual ~EditorLoader();

		Glory::EditorCreateInfo LoadEditor(Game& game, EngineLoader& engineLoader);
		void Unload();


	private:
		void LoadModuleMetadata(Glory::EditorCreateInfo& editorCreateInfo, const std::string& name);

	private:
		std::vector<HMODULE> m_Libs;
	};
}
