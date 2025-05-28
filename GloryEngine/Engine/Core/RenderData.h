#pragma once
#include "LayerMask.h"
#include "UUID.h"

#include <glm/glm.hpp>
#include <ReflectGen.h>

REFLECTABLE_ENUM_NS(Glory, Alignment, Left, Center, Right)

namespace Glory
{
	class MeshData;
	class MaterialData;

	struct ObjectData
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
		uint64_t SceneID;
		uint64_t ObjectID;
	};

	struct RenderData
	{
	public:
		// Mesh
		UUID m_MeshID;
		UUID m_SceneID;
		UUID m_ObjectID;

		// World matrices
		glm::mat4 m_World;
		//size_t m_NumInstances;
		//
		// Material
		UUID m_MaterialID;
		LayerMask m_LayerMask;
		bool m_DepthWrite{ true };
	};

	struct TextRenderData
	{
		/* Font */
		UUID m_FontID;
		UUID m_SceneID;
		UUID m_ObjectID;
		bool m_TextDirty;

		/* Settings */
		float m_Scale;
		Alignment m_Alignment;
		float m_TextWrap;

		/* Text */
		std::string m_Text;
		glm::vec4 m_Color;

		/* World matrix */
		glm::mat4 m_World;

		LayerMask m_LayerMask;

		glm::vec2 m_Offsets{};
		bool m_Append{ false };
	};
}
