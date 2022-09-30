#pragma once
#include "IToolChain.h"
#include <ImGuizmo.h>

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

	private:
		static const ImGuizmo::OPERATION OPERATIONS[];
		static const size_t OPERATIONS_COUNT;

		static const ImGuizmo::MODE MODES[];
		static const size_t MODES_COUNT;
	};
}
