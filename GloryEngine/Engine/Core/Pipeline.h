#pragma once
#include "GPUResource.h"

namespace Glory
{
	class PipelineData;
	class Engine;
	class Shader;

	/** @brief Compiled GPU pipeline */
    class Pipeline : public GPUResource
    {
	public:
		/** @brief Constructor */
		Pipeline(PipelineData* pPipelineData);
		/** @brief Destructor */
		virtual ~Pipeline();

		/** @brief Use this pipeline */
		virtual void Use() = 0;
		virtual void UnUse() = 0;

	protected:
		/** @brief Initialize the pipeline */
		virtual void Initialize() = 0;
		/** @brief Add a shader to the pipeline
		 * @param pShader Shader to add
		 */
		void AddShader(Shader* pShader);

		/** @brief Clear all shaders from this pipeline */
		void Clear();

	protected:
		PipelineData* m_pPipelineData;
		std::vector<Shader*> m_pShaders;
		bool m_Complete;

	private:
		friend class GPUResourceManager;
    };
}
