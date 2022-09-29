#pragma once
#include "IToolChain.h"
#include <map>
#include <vector>
#include <imgui.h>

namespace Glory::Editor
{
	enum class ToolChainPosition
	{
		Left,
		Center,
		Right,
	};

	class Toolbar
	{
	public:
		Toolbar(float toolbarSize);
		virtual ~Toolbar();

		static void AddToolChain(const ToolChainPosition& toolChain, IToolChain* pToolChain);

	private:
		void Paint();

		void LeftToolchain();
		void CenterToolchain();
		void RightToolchain();

		void DrawSubBar(float startPos, const ToolChainPosition& toolChain);

	private:
		friend class MainEditor;
		const float TOOLBAR_SIZE;
		float m_ToolbarLength;
		static const float BUTTON_SPACING;

		static std::map<ToolChainPosition, std::vector<IToolChain*>> m_ToolChains;
		std::map<ToolChainPosition, float> m_LastFrameToolBarLengths;
	};
}
