#pragma once
#include "MaterialData.h"
#include "Shader.h"
#include "RenderData.h"
#include "Texture.h"
#include "Buffer.h"

namespace Glory
{
	class Material : public GPUResource
	{
	public:
		Material(MaterialData* pMaterialData);
		virtual ~Material();

		virtual void Use() = 0;
		void SetProperties();
		virtual void SetPropertiesExtra() = 0;

	public: // Property setters
		void SetUBO(UniformBufferObjectTest ubo);

		// All of these need to go, properties should be set using the material data/material instance data class linked to this material
		virtual void SetFloat(const std::string& name, float value) const = 0;
		virtual void SetInt(const std::string& name, int value) const = 0;
		virtual void SetUInt(const std::string& name, uint32_t value) const = 0;
		virtual void SetIntArray(const std::string& name, int size, int* value) const = 0;
		virtual void SetBool(const std::string& name, bool value) const = 0;
		virtual void SetVec2(const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetVec3(const std::string& name, const glm::vec3& value) const = 0;
		virtual void SetUVec3(const std::string& name, const glm::uvec3& value) const = 0;
		virtual void SetVec4(const std::string& name, const glm::vec4& value) const = 0;
		virtual void SetDouble(const std::string& name, double value) const = 0;
		virtual void SetMatrix3(const std::string& name, const glm::mat3& value) const = 0;
		virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const = 0;
		virtual void SetTexture(const std::string& name, Texture* value) = 0;
		//virtual void SetTexture(const std::string& name, const GLuint& textureID) = 0;

	protected:
		virtual void Initialize() = 0;
		void AddShader(Shader* pShader);

		virtual Buffer* CreatePropertiesBuffer(size_t size) = 0;

		void Clear();

	protected:
		MaterialData* m_pMaterialData;
		std::vector<Shader*> m_pShaders;
		UniformBufferObjectTest m_UBO;

		Buffer* m_pPropertiesBuffer;
		bool m_Complete;

	private:
		friend class GPUResourceManager;
	};
}
