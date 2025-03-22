#pragma once
#include <EditorWindow.h>

namespace Glory
{
	class RenderTexture;
}

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

		UUID m_CurrentCubemap;
		bool m_Generate;
		RenderTexture* m_pIrradianceResult;

		std::string m_OutputPath;
		std::string m_Filename;

		float* m_pFaces[6];
	};
}
