#pragma once
#include <Mesh.h>
#include <GL/glew.h>
#include "GLBuffer.h"

namespace Glory
{
	class GLMesh : public Mesh
	{
    public:
        GLMesh(size_t vertexCount, size_t indexCount, InputRate inputRate, size_t binding, size_t stride, const std::vector<AttributeType>& attributeTypes);
        virtual ~GLMesh();
        virtual void CreateBindingAndAttributeData() override;
        virtual void Bind() override;

    private:
        void SetupAttributes();

    private:
        GLuint m_VertexArrayID;
	};
}
