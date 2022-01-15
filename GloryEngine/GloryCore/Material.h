#pragma once
#include "MaterialData.h"
#include "Shader.h"
#include "RenderData.h"
#include "Texture.h"

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
		void SetProperty(MaterialPropertyData* pProperty);

	protected:
		virtual void Initialize() = 0;
		void AddShader(Shader* pShader);

		virtual void SetFloat(const std::string& name, float value) const = 0;
		virtual void SetInt(const std::string& name, int value) const = 0;
		virtual void SetIntArray(const std::string& name, int size, int* value) const = 0;
		virtual void SetBool(const std::string& name, bool value) const = 0;
		virtual void SetVec2(const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetVec3(const std::string& name, const glm::vec3& value) const = 0;
		virtual void SetVec4(const std::string& name, const glm::vec4& value) const = 0;
		virtual void SetDouble(const std::string& name, double value) const = 0;
		virtual void SetMatrix3(const std::string& name, const glm::mat3& value) const = 0;
		virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const = 0;
		virtual void SetTexture(const std::string& name, Texture* value) = 0;
		//virtual void SetTexture(const std::string& name, const GLuint& textureID) = 0;

	protected:
		MaterialData* m_pMaterialData;
		std::vector<Shader*> m_pShaders;
		UniformBufferObjectTest m_UBO;

		std::vector<MaterialPropertyData> m_FrameProperties;

	private:
		friend class GPUResourceManager;
	};
}
