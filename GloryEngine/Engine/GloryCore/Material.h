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
		void SetObjectData(const ObjectData& data);

		virtual void SetTexture(const std::string& name, Texture* value) = 0;

	public: // Property setters
		// All of these need to go, properties should be set using the material data/material instance data class linked to this material
		virtual void SetFloat(const std::string& name, float value) const = 0;
		virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const = 0;

	protected:
		virtual void Initialize() = 0;
		void AddShader(Shader* pShader);

		virtual Buffer* CreatePropertiesBuffer(uint32_t size) = 0;
		virtual Buffer* CreateMVPBuffer() = 0;

		void Clear();

	protected:
		MaterialData* m_pMaterialData;
		std::vector<Shader*> m_pShaders;

		Buffer* m_pPropertiesBuffer;
		static Buffer* m_pMVPBuffer;
		bool m_Complete;

	private:
		friend class GPUResourceManager;
	};
}
