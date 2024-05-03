#pragma once
#include "GPUResource.h"

namespace Glory
{
	class PipelineData;
	class Engine;
	class Shader;

    class Pipeline : public GPUResource
    {
	public:
		Pipeline(PipelineData* pPipelineData);
		virtual ~Pipeline();

		virtual void Use() = 0;

	protected:
		virtual void Initialize() = 0;
		void AddShader(Shader* pShader);

		void Clear();

	protected:
		PipelineData* m_pPipelineData;
		std::vector<Shader*> m_pShaders;
		bool m_Complete;

	private:
		friend class GPUResourceManager;
    };
}
