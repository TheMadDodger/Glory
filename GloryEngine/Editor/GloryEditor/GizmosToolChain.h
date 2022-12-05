#pragma once
#include "IToolChain.h"
#include <string>

namespace ImGuizmo
{
	enum OPERATION;
	enum MODE;
}

namespace Glory::Editor
{
	class GizmosToolChain : public IToolChain
	{
	public:
		GizmosToolChain();
		virtual ~GizmosToolChain();

	private:
		virtual void DrawToolchain(float& cursor, const ImVec2& maxButtonSize);

		void DrawOperations(float& cursor, const ImVec2& maxButtonSize);
		void DrawModes(float& cursor, const ImVec2& maxButtonSize);
	};
}
