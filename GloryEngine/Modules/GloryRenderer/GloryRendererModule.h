#pragma once
#include "renderer_visibility.h"

#include "GloryRenderer.h"
#include "GloryRendererData.h"
#include "AssetReference.h"

#include <Version.h>
#include <SettingsContainer.h>
#include <Module.h>

#include <glm/glm.hpp>

GLORY_MODULE_H(GLORY_RENDERER_API)

namespace Glory
{
	struct GloryRendererSettings
	{
		GloryRendererSettings() { }

		REFLECTABLE_DESCRIPTIVE(GloryRendererSettings,
			(ResourceReference<PipelineData>, m_LinesPipeline, "Lines Pipeline", ""),
			(ResourceReference<PipelineData>, m_SSAOPrepassPipeline, "SSAO Prepass Pipeline", ""),
			(ResourceReference<PipelineData>, m_SSAOBlurPipeline, "SSAO Blur Pipeline", ""),
			(ResourceReference<PipelineData>, m_TextPipeline, "Text Pipeline", ""),
			(ResourceReference<PipelineData>, m_DisplayCopyPipeline, "Display Copy Pipeline", ""),
			(ResourceReference<PipelineData>, m_SkyboxPipeline, "Skybox Pipeline", ""),
			(ResourceReference<PipelineData>, m_ShadowsPipeline, "Shadows Pipeline", ""),
			(ResourceReference<PipelineData>, m_ShadowsTransparentTexturePipeline, "Shadows Transparent Textured Pipeline", ""),
			(ResourceReference<PipelineData>, m_ClusterPipeline, "Cluster Generator", ""),
			(ResourceReference<PipelineData>, m_ClusterCullLightPipeline, "Cluster Cull Light", ""),
			(ResourceReference<PipelineData>, m_PickingPipeline, "Picking", ""),
			(ResourceReference<PipelineData>, m_SSAOPostpassPipeline, "SSAO Postpass", ""),
			(ResourceReference<PipelineData>, m_SSAOVisPipeline, "SSAO Visualizer", ""),
			(ResourceReference<PipelineData>, m_ObjectIDVisPipeline, "ObjectID Visualizer", ""),
			(ResourceReference<PipelineData>, m_DepthVisPipeline, "Depth Visualizer", ""),
			(ResourceReference<PipelineData>, m_LightComplexityVisPipeline, "Light Complexity Visualizer", "")
		);
	};

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

		GloryRenderer* CreateSecondaryRenderer(size_t imageCount);

		GLORY_MODULE_VERSION_H(0, 1, 0);

	private:
		virtual void RegisterTypes() override;
		virtual void InitializeSettings() override;
		virtual void Cleanup() override;
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Update() override;
		virtual void Draw() override;
		virtual void Preload() override;

		virtual void OnProcessData() override;

	private:
		SettingsContainer<GloryRendererSettings> m_Settings;

		GloryRenderer m_Renderer;
		static constexpr size_t MaxSecondaryRenderers = 10;
		size_t m_NextSecundaryRenderer = 0;
		std::array<GloryRenderer, MaxSecondaryRenderers> m_SecondaryRenderers;

		std::vector<UUID> m_PipelineOrder;

		bool m_FirstPipelineCacheCheck = true;
	};
}
