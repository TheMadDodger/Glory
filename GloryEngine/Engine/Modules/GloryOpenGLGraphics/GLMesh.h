#pragma once
#include <Mesh.h>
#include <GL/glew.h>
#include "GLBuffer.h"

namespace Glory
{
	class GLMesh : public Mesh
	{
    public:
        GLMesh(uint32_t vertexCount, uint32_t indexCount, InputRate inputRate, uint32_t binding, uint32_t stride, const std::vector<AttributeType>& attributeTypes);
        virtual ~GLMesh();
        virtual void CreateBindingAndAttributeData() override;
        virtual void Bind() override;

    private:
        void SetupAttributes();

    private:
        GLuint m_VertexArrayID;
	};
}
