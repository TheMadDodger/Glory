#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace Glory
{
	struct Vertex
	{
		glm::vec2 Pos;
		glm::vec3 Color;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(const std::vector<Vertex>& vertices);
		virtual ~VertexBuffer();

	private:
		const std::vector<Vertex> m_Vertices;
	};
}
