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

	struct LineVertex
	{
		glm::vec3 Pos;
		glm::vec4 Color;
	};

	struct DefaultVertex3D
	{
		glm::vec3 Pos;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
		glm::vec2 TexCoord;
		glm::vec4 Color;
	};
}