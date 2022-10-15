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

	private:
		static const ImGuizmo::OPERATION OPERATIONS[];
		static const std::string OPERATION_TEXTURES[];
		static const size_t OPERATIONS_COUNT;

		static const ImGuizmo::MODE MODES[];
		static const std::string MODE_TEXTURES[];
		static const size_t MODES_COUNT;
	};
}
