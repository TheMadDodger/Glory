#pragma once
#include "GloryEditor.h"
#include <string>

namespace Glory::Editor
{
	class QuitPopup
	{
	public:
		GLORY_EDITOR_API static void Open(const char* text);

	private:
		static void Draw();

	private:
		friend class MainEditor;


	private:
		static bool m_Open;
		static std::string m_Text;
		QuitPopup() = delete;
	};
}
