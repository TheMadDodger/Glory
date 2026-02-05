#pragma once
#include "GloryRenderer.h"
#include "GloryRendererData.h"

#include <RendererModule.h>

#include <glm/glm.hpp>

GLORY_MODULE_H

namespace Glory
{
	class GraphicsDevive;

	class GloryRendererModule : public RendererModule
	{
	public:
		GloryRendererModule();
		virtual ~GloryRendererModule();

		virtual void CollectReferences(std::vector<UUID>& references) override;

		void CheckCachedPipelines(GraphicsDevice* pDevice);

		GLORY_MODULE_VERSION_H(0, 1, 0);

	private:
		virtual void Cleanup() override;
		virtual void Initialize() override;
		virtual void OnPostInitialize() override;
		virtual void Update() override;
		virtual void Draw() override;
		virtual void LoadSettings(ModuleSettings& settings) override;
		virtual void Preload() override;

	private:
		GloryRenderer m_Renderer;
	};
}
