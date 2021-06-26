#pragma once

namespace Glory::Editor
{
	class MainEditor
	{
	public:
		MainEditor();
		virtual ~MainEditor();

		void Initialize();
		void Destroy();
		void Paint();

	private:
		void CreateDefaultMainMenuBar();
	};
}
