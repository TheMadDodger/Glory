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
        virtual void SetPropertiesExtra() override;
        virtual Buffer* CreatePropertiesBuffer(size_t size) override;

    private:
        GLuint CreateUniformBuffer(const std::string& name, GLuint bufferSize, GLuint bindingIndex);
        void SetUniformBuffer(GLuint bufferID, void* data, GLuint size);
        //void SetTexture(const std::string& name, GLTexture* pTexture);

    public: // Set shader vars
        virtual void SetFloat(const std::string& name, float value) const override;
        virtual void SetInt(const std::string& name, int value) const override;
        virtual void SetUInt(const std::string& name, uint32_t value) const override;
        virtual void SetIntArray(const std::string& name, int size, int* value) const override;
        virtual void SetBool(const std::string& name, bool value) const override;
        virtual void SetVec2(const std::string& name, const glm::vec2& value) const override;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) const override;
        virtual void SetUVec3(const std::string& name, const glm::uvec3& value) const override;
        virtual void SetVec4(const std::string& name, const glm::vec4& value) const override;
        virtual void SetDouble(const std::string& name, double value) const override;
        virtual void SetMatrix3(const std::string& name, const glm::mat3& value) const override;
        virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const override;
        virtual void SetTexture(const std::string& name, Texture* value) override;
        //virtual void SetTexture(const std::string& name, const GLuint& textureID);

    private:
        GLuint m_ProgramID;
        std::vector<GLuint> m_UniformBufferObjects;
        GLuint m_UBOID;
        GLuint m_TextureCounter;
    };
}
