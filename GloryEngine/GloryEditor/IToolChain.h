#pragma once
#include <imgui.h>

namespace Glory::Editor
{
	class IToolChain
	{
	public:
		virtual void DrawToolchain(float& cursor, const ImVec2& maxButtonSize) = 0;
	};
}