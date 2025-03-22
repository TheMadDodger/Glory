#pragma once
#include "MaterialData.h"
#include "Shader.h"
#include "RenderData.h"
#include "Texture.h"
#include "Buffer.h"

namespace Glory
{
	class Engine;
	class Pipeline;

	/** @brief Compiled GPU material */
	class Material : public GPUResource
	{
	public:
		/** @brief Constructor */
		Material(MaterialData* pMaterialData);
		/** @brief Destructor */
		virtual ~Material();

		/** @brief Use this material and its pipeline */
		virtual void Use() = 0;
		/** @brief Update properties buffer and samplers */
		void SetProperties(Engine* pEngine);
		/** @brief Update object data for rendering objects */
		void SetObjectData(const ObjectData& data);

		/** @brief Update only samplers */
		void SetSamplers(Engine* pEngine);
		/** @brief Update only propertties buffer and texture bits */
		void SetPropertiesBuffer(Engine* pEngine);

		/** @brief Set texture on material */
		virtual void SetTexture(const std::string& name, Texture* value) = 0;
		/** @brief Set cubemap texture on material */
		virtual void SetCubemapTexture(const std::string& name, Texture* value) = 0;
		/** @brief Set float on material */
		virtual void SetFloat(const std::string& name, float value) const = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) const = 0;
		/** @brief Set 4x4 matrix on material */
		virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const = 0;
		/** @brief Resets the texture counter */
		virtual void ResetTextureCounter() = 0;

	protected:
		virtual Buffer* CreatePropertiesBuffer(uint32_t size) = 0;
		virtual Buffer* CreateHasTexturesBuffer() = 0;
		virtual Buffer* CreateMVPBuffer() = 0;

	protected:
		MaterialData* m_pMaterialData;
		Pipeline* m_pPipeline;

		Buffer* m_pPropertiesBuffer;
		Buffer* m_pHasTexturesBuffer;
		static Buffer* m_pMVPBuffer;
		bool m_Complete;
		uint64_t m_TextureSetBits;

	private:
		friend class GPUResourceManager;
	};
}
