#pragma once
#include "GloryRenderer.h"
#include "GloryRendererData.h"

#include <Module.h>

#include <glm/glm.hpp>

GLORY_MODULE_H

namespace Glory
{
	class GraphicsDevive;

	class GloryRendererModule : public Module
	{
	public:
		GloryRendererModule();
		virtual ~GloryRendererModule();

		virtual void CollectReferences(std::vector<UUID>& references) override;

		void CheckCachedPipelines(GraphicsDevice* pDevice);

		const std::vector<UUID>& PipelineOrder() const;
		void SetPipelineOrder(std::vector<UUID>&& pipelineOrder);

		const std::type_info& GetModuleType() override;

		GLORY_MODULE_VERSION_H(0, 1, 0);

	private:
		virtual void Cleanup() override;
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Draw() override;
		virtual void LoadSettings(ModuleSettings& settings) override;
		virtual void Preload() override;

		virtual void OnProcessData() override;

	private:
		GloryRenderer m_Renderer;

		std::vector<UUID> m_PipelineOrder;
	};
}
