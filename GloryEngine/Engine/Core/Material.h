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
		/** @brief Update properties buffer */
		void SetProperties(Engine* pEngine);
		/** @brief Update object data for rendering objects */
		void SetObjectData(const ObjectData& data);

		/** @brief Set texture on material */
		virtual void SetTexture(const std::string& name, Texture* value) = 0;
		/** @brief Set float on material */
		virtual void SetFloat(const std::string& name, float value) const = 0;
		/** @brief Set 4x4 matrix on material */
		virtual void SetMatrix4(const std::string& name, const glm::mat4& value) const = 0;

	protected:
		virtual Buffer* CreatePropertiesBuffer(uint32_t size) = 0;
		virtual Buffer* CreateMVPBuffer() = 0;

	protected:
		MaterialData* m_pMaterialData;
		Pipeline* m_pPipeline;

		Buffer* m_pPropertiesBuffer;
		static Buffer* m_pMVPBuffer;
		bool m_Complete;

	private:
		friend class GPUResourceManager;
	};
}
