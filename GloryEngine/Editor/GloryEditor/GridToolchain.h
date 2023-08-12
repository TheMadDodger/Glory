#pragma once
#include "IToolChain.h"

#include <glm/glm.hpp>
#include <ImGuizmo.h>

namespace Glory::Editor
{
	class GridToolchain : public IToolChain
	{
	public:
		GridToolchain();

		const float* GetSnap(ImGuizmo::OPERATION op);

	private:
		void DrawToolchain(float& cursor, const ImVec2& maxButtonSize) override;

	private:
		glm::vec3 m_SnappingGrids[3];
		bool m_SnappingEnabled[3];
	};
}
