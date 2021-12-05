#pragma once
#include <glm/glm.hpp>

namespace Glory
{
	struct Vertex
	{
		glm::vec2 Pos;
		glm::vec3 Color;
	};

	struct VertexPosColorTex
	{
		glm::vec2 Pos;
		glm::vec3 Color;
		glm::vec2 TexCoord;
	};

	struct Vertex3DColorTex
	{
		glm::vec3 Pos;
		glm::vec3 Color;
		glm::vec2 TexCoord;
	};

	struct Vertex3DTex
	{
		glm::vec3 Pos;
		glm::vec2 TexCoord;
	};
}