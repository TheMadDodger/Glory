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
		virtual void SetProperties() = 0;
		virtual void SetPropertiesNoUBO() = 0;

		//virtual void SetTexture(const std::string& name, TextureData* pTexture);
		//virtual void SetTexture(const std::string& name, const GLuint& textureID);

	public: // Property setters
		void SetUBO(UniformBufferObjectTest ubo);
		void SetTexture(ImageData* pImageData);
		void SetTexture(Texture* pTexture);

	protected:
		virtual void Initialize() = 0;
		void AddShader(Shader* pShader);

	protected:
		MaterialData* m_pMaterialData;
		std::vector<Shader*> m_pShaders;
		UniformBufferObjectTest m_UBO;
		Texture* m_pTexture;

	private:
		friend class GPUResourceManager;
	};
}
