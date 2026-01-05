#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
	class EnvironmentGenerator : public EditorWindowTemplate<EnvironmentGenerator>
	{
	public:
		EnvironmentGenerator();
		~EnvironmentGenerator();

	private:
		virtual void OnGUI() override;
		virtual void OnOpen() override;
		virtual void OnClose() override;

		void EnvironmentPass();

		void Initialize();

		UUID m_CurrentCubemap;
		bool m_Generate;
		RenderPassHandle m_IrradianceRenderPass;
		PipelineHandle m_IrradiancePipeline;
		DescriptorSetHandle m_CubemapSet;

		TextureHandle m_CubemapFaces[6];

		std::string m_OutputPath;
		std::string m_Filename;
		static bool m_Initialized;

		float* m_pFaces[6];
	};
}
