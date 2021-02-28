#pragma once
#include "VertexDefinitions.h"
#include <vector>
#include "MeshData.h"

namespace Glory
{
	class Mesh
	{
	public:
		Mesh(size_t vertexCount, size_t indexCount, InputRate inputRate, size_t binding, size_t stride, const std::vector<AttributeType>& attributeTypes);
		virtual ~Mesh();

		virtual void CreateBindingAndAttributeData() = 0;
		uint32_t GetVertexCount() const;
		uint32_t GetIndexCount() const;

	protected:
		void GetNextOffset(const AttributeType& atributeType, uint32_t& offest);

	protected:
		const InputRate m_InputRate;
		const size_t m_Binding;
		const size_t m_Stride;
		const size_t m_VertexCount;
		const size_t m_IndexCount;
		const std::vector<AttributeType> m_AttributeTypes;
	};
}