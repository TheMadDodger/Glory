#pragma once
#include "GLTexture.h"

#include <Material.h>

#include <GL/glew.h>

namespace Glory
{
    class OGLPipeline;

    class OGLMaterial : public Material
    {
    public:
        OGLMaterial(MaterialData* pMaterialData);
        virtual ~OGLMaterial();

        virtual void Use() override;
        void SetTexture(const std::string& name, GLuint id);

        OGLPipeline* Pipeline() const;

    private:
        virtual Buffer* CreatePropertiesBuffer(uint32_t size) override;
        virtual Buffer* CreateMVPBuffer() override;
        virtual void SetTexture(const std::string& name, Texture* value) override;

    public: // Set shader vars
        virtual void SetFloat(const std::string& name, float value) const override;
        virtual void SetFloat3(const std::string& name, const glm::vec3& value) const override;
        virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const override;
        //virtual void SetTexture(const std::string& name, const GLuint& textureID);

    private:
        std::vector<GLuint> m_UniformBufferObjects;
        GLuint m_UBOID;
        GLuint m_TextureCounter;
    };
}
