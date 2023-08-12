#pragma once
#include "VertexDefinitions.h"
#include <vector>
#include "MeshData.h"
#include "Buffer.h"

namespace Glory
{
	class Mesh : public GPUResource
	{
	public:
		Mesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, const std::vector<AttributeType>& attributeTypes);
		Mesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes);
		virtual ~Mesh();

		virtual void CreateBindingAndAttributeData() = 0;
		virtual void Bind() = 0;
		uint32_t GetVertexCount() const;
		uint32_t GetIndexCount() const;

		const PrimitiveType GetPrimitiveType() const;

		void SetBuffers(Buffer* pVertexBuffer, Buffer* pIndexBuffer);

	protected:
		void GetNextOffset(const AttributeType& atributeType, uint32_t& offest);

	protected:
		const InputRate m_InputRate;
		const uint32_t m_Binding;
		const uint32_t m_Stride;
		const uint32_t m_VertexCount;
		const uint32_t m_IndexCount;
		const PrimitiveType m_PrimitiveType;
		const std::vector<AttributeType> m_AttributeTypes;

		Buffer* m_pVertexBuffer;
		Buffer* m_pIndexBuffer;
	};
}