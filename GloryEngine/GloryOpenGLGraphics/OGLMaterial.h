#pragma once
#include <Material.h>
#include <GL/glew.h>
#include "GLTexture.h"

namespace Glory
{
    class OGLMaterial : public Material
    {
    public:
        OGLMaterial(MaterialData* pMaterialData);
        virtual ~OGLMaterial();

        virtual void Use() override;
        void SetTexture(const std::string& name, GLuint id);

    private:
        virtual void Initialize() override;
        virtual void SetProperties() override;
        virtual void SetPropertiesNoUBO() override;

    private:
        GLuint CreateUniformBuffer(const std::string& name, GLuint bufferSize, GLuint bindingIndex);
        void SetUniformBuffer(GLuint bufferID, void* data, GLuint size);
        void SetTexture(const std::string& name, GLTexture* pTexture);

    private:
        GLuint m_ProgramID;
        std::vector<GLuint> m_UniformBufferObjects;
        GLuint m_UBOID;
        GLuint m_TextureCounter;
    };
}
